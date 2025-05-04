#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "qr.h"

static void
print_usage_and_fail(char *exe)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", exe);
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "    -t TEXT    Encode the given TEXT. Cannot be combined with -f.\n");
    fprintf(stderr, "    -f FILE    Encode the content of FILE. Cannot be combined with -t.\n");
    fprintf(stderr, "    -l LEVEL   Force error correction level from 0 (Low) to 3 (High).\n");
    fprintf(stderr, "    -v VERSION Force QR version from 1 to 40.\n");
    fprintf(stderr, "    -m MASK    Force mask pattern from 0 to 7.\n");
    fprintf(stderr, "    -a         Print QR code in ASCII.\n");
    fprintf(stderr, "    -d         Print debugging messages to STDERR.\n");
    fprintf(stderr, "If neither -t nor -f is specified, encodes the data read from STDIN.\n");
    exit(1);
}

static QROptions
parse_options(int32_t argc, char **argv)
{
    QROptions options = {};
    options.forcedLevel = LEVEL_INVALID;
    options.forcedVersion = VERSION_INVALID;
    options.forcedMask = MASK_INVALID;
    options.format = OF_ANSI_COLOR;
    options.isDebug = false;

    char *exe = argv[0];
    char *text = NULL;
    char *filename = NULL;

    for (int32_t i = 1; i < argc; i++) {
        char *option = argv[i];
        if (strlen(option) != 2 || option[0] != '-') {
            fprintf(stderr, "Unknown option: %s\n", option);
            print_usage_and_fail(exe);
        }
        switch (option[1]) {
            case 't': {
                if (filename != NULL) {
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
                filename = argv[i];
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
            case 'd': {
                options.isDebug = true;
            } break;
            case 'a': {
                options.format = OF_ASCII;
            } break;
            default: {
                fprintf(stderr, "Unknown option: %c\n", argv[i][1]);
                print_usage_and_fail(exe);
            }
        }
    }

    if (text != NULL) {
        size_t textLen = strlen(text);
        strncpy(options.text, text, textLen);
        options.textLen = (int32_t)textLen;
    }
    else if (filename != NULL) {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            fprintf(stderr, "Failed to open file '%s': %s\n", filename, strerror(errno));
            exit(1);
        }
        size_t fileSize = fread(options.text, sizeof(options.text[0]), MAX_TEXT_LEN, file);
        if (ferror(file) != 0) {
            fprintf(stderr, "Failed to read from file '%s': %s\n", filename, strerror(errno));
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
        FILE *file = stdin;
        size_t fileSize = fread(options.text, sizeof(options.text[0]), MAX_TEXT_LEN, file);
        if (ferror(file) != 0) {
            fprintf(stderr, "Failed to read from STDIN: %s\n", strerror(errno));
            exit(1);
        }
        options.text[fileSize] = '\0';
        options.textLen = (int32_t)fileSize;
    }

    ASSERT(options.textLen > 0);

    return options;
}

int32_t
main(int32_t argc, char **argv)
{
    QROptions options = parse_options(argc, argv);
    QR qr = qr_encode(&options);

    printf("\n");
    qr_print(stdout, &qr, options.format);
    printf("\n");

    return 0;
}
