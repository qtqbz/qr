#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "qr.h"

__attribute__((noreturn)) internal void
print_usage_and_fail(char *exe)
{
    fprintf(stderr, "Usage: %s [OPTION]...\n", exe);
    fprintf(stderr, "Where OPTION is one of the following:\n");
    fprintf(stderr, "    -t TEXT    Encode the given TEXT. Cannot be combined with -f.\n");
    fprintf(stderr, "    -f FILE    Encode the content of FILE. Cannot be combined with -t.\n");
    fprintf(stderr, "    -l LEVEL   Force error correction level, where VERSION is a number from 0 (Low) to 3 (High).\n");
    fprintf(stderr, "    -v VERSION Force QR version, where VERSION is a number from 1 to 40.\n");
    fprintf(stderr, "    -m MASK    Force mask pattern, where MASK is a number from 0 to 7.\n");
    fprintf(stderr, "    -o FORMAT  Output format, where FORMAT is one of: ANSI, ASCII, UTF8, UTF8Q. Defaults to UTF8.\n");
    fprintf(stderr, "    -d         Print debugging messages to STDERR.\n");
    fprintf(stderr, "If neither -t nor -f is specified, encodes the data read from STDIN.\n");
    exit(1);
}

internal OutputFormat
parse_output_format(char *exe, char *outputFormatString)
{
    if (strcmp(outputFormatString, "ANSI") == 0) {
        return OF_ANSI;
    }
    if (strcmp(outputFormatString, "ASCII") == 0) {
        return OF_ASCII;
    }
    if (strcmp(outputFormatString, "UTF8") == 0) {
        return OF_UTF8;
    }
    if (strcmp(outputFormatString, "UTF8Q") == 0) {
        return OF_UTF8Q;
    }
    fprintf(stderr, "Invalid output format: %s\n", outputFormatString);
    print_usage_and_fail(exe);
}

internal QROptions
parse_options(int32_t argc, char **argv)
{
    QROptions options = {};
    options.forcedLevel = LEVEL_INVALID;
    options.forcedVersion = VERSION_INVALID;
    options.forcedMask = MASK_INVALID;
    options.outputFormat = OF_UTF8;
    options.isDebug = false;

    char *exe = argv[0];
    char *text = NULL;
    char *filepath = NULL;

    for (int32_t i = 1; i < argc; i++) {
        char *option = argv[i];
        if (strlen(option) != 2 || option[0] != '-') {
            fprintf(stderr, "Unknown option: %s\n", option);
            print_usage_and_fail(exe);
        }
        switch (option[1]) {
            case 't': {
                if (filepath != NULL) {
                    fprintf(stderr, "-t cannot be combined with -f\n");
                    print_usage_and_fail(exe);
                }
                if (++i >= argc) {
                    fprintf(stderr, "Missing TEXT\n");
                    print_usage_and_fail(exe);
                }
                text = argv[i];
            } break;
            case 'f': {
                if (text != NULL) {
                    fprintf(stderr, "-f cannot be combined with -t\n");
                    print_usage_and_fail(exe);
                }
                if (++i >= argc) {
                    fprintf(stderr, "Missing FILE\n");
                    print_usage_and_fail(exe);
                }
                filepath = argv[i];
            } break;
            case 'l': {
                if (++i >= argc) {
                    fprintf(stderr, "Missing LEVEL\n");
                    print_usage_and_fail(exe);
                }
                char *levelString = argv[i];
                intmax_t level = strtoimax(levelString, NULL, 10);
                if (level < ECL_LOW || level > ECL_HIGH) {
                    fprintf(stderr, "Invalid error correction level: %s\n", levelString);
                    print_usage_and_fail(exe);
                }
                options.forcedLevel = (ErrorCorrectionLevel)level;
            } break;
            case 'v': {
                if (++i >= argc) {
                    fprintf(stderr, "Missing VERSION\n");
                    print_usage_and_fail(exe);
                }
                char *versionString = argv[i];
                intmax_t version = strtoimax(versionString, NULL, 10) - 1;
                if (version < MIN_VERSION || version > MAX_VERSION) {
                    fprintf(stderr, "Invalid version: %s\n", versionString);
                    print_usage_and_fail(exe);
                }
                options.forcedVersion = (int32_t)version;
            } break;
            case 'm': {
                if (++i >= argc) {
                    fprintf(stderr, "Missing MASK\n");
                    print_usage_and_fail(exe);
                }
                char *maskString = argv[i];
                intmax_t mask = strtoimax(maskString, NULL, 10);
                if (mask < MIN_MASK || mask > MAX_MASK) {
                    fprintf(stderr, "Invalid mask: %s\n", maskString);
                    print_usage_and_fail(exe);
                }
                options.forcedMask = (int32_t)mask;
            } break;
            case 'o': {
                if (++i >= argc) {
                    fprintf(stderr, "Missing FORMAT\n");
                    print_usage_and_fail(exe);
                }
                char *outputFormatString = argv[i];
                options.outputFormat = parse_output_format(exe, outputFormatString);
            } break;
            case 'd': {
                options.isDebug = true;
            } break;
            default: {
                fprintf(stderr, "Unknown option: %c\n", option[1]);
                print_usage_and_fail(exe);
            }
        }
    }

    if (text != NULL) {
        size_t textLen = strlen(text);
        strncpy(options.text, text, textLen);
        options.textLen = (int32_t)textLen;
    }
    else if (filepath != NULL) {
        FILE *file = fopen(filepath, "r");
        if (file == NULL) {
            fprintf(stderr, "Failed to open file '%s': %s\n", filepath, strerror(errno));
            exit(1);
        }
        size_t fileSize = fread(options.text, sizeof(options.text[0]), MAX_TEXT_LEN + 1, file);
        if (ferror(file) != 0) {
            fprintf(stderr, "Failed to read from file '%s': %s\n", filepath, strerror(errno));
            exit(1);
        }
        if (fileSize > MAX_TEXT_LEN) {
            fprintf(stderr, "File '%s' exceeds the maximum size of %d bytes\n", filepath, MAX_TEXT_LEN);
            exit(1);
        }
        fclose(file);
        options.text[fileSize] = '\0';
        options.textLen = (int32_t)fileSize;
    }
    else {
        if (isatty(STDIN_FILENO)) {
            printf("Enter text to encode (Ctrl+D to finish): ");
            fflush(stdout);
        }
        size_t inputSize = fread(options.text, sizeof(options.text[0]), MAX_TEXT_LEN + 1, stdin);
        if (ferror(stdin) != 0) {
            fprintf(stderr, "Failed to read from STDIN: %s\n", strerror(errno));
            exit(1);
        }
        if (inputSize > MAX_TEXT_LEN) {
            fprintf(stderr, "Input exceeds the maximum length of %d characters\n", MAX_TEXT_LEN);
            exit(1);
        }
        options.text[inputSize] = '\0';
        options.textLen = (int32_t)inputSize;
    }

    ASSERT(options.textLen > 0);

    return options;
}

int32_t
main(int32_t argc, char **argv)
{
    QROptions options = parse_options(argc, argv);
    QR qr = qr_encode(&options);
    qr_print(stdout, &qr, options.outputFormat);
    return 0;
}
