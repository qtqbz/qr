#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.c"
#include "qr.c"
#include "bv.c"
#include "gf256.c"

typedef struct CommandLineArguments CommandLineArguments;
struct CommandLineArguments
{
    const char *text;
    bool verbose;
};

static void
print_usage(char *exe)
{
    fprintf(stderr, "Usage: %s [OPTIONS] <YOUR TEXT>\n", exe);
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "\t-v verbose\n");
}

static CommandLineArguments
parse_args(int32_t argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    CommandLineArguments args = {};

    for (int32_t i = 1; i < argc - 1; i++) {
        if (argv[i][0] != '-') {
            print_usage(argv[0]);
            exit(1);
        }
        for (int32_t j = 1; argv[i][j] != '\0'; j++) {
            switch (argv[i][j]) {
                case 'v': {
                    args.verbose = true;
                } break;
                default: {
                    fprintf(stderr, "Unknown flag: %c\n", argv[i][1]);
                    print_usage(argv[0]);
                    exit(1);
                }
            }
        }
    }

    args.text = argv[argc - 1];

    return args;
}

int32_t
main(int32_t argc, char **argv)
{
    CommandLineArguments args = parse_args(argc, argv);
    ASSERT(args.text != NULL);

    const char *text = args.text;
    int32_t textCharCount = (int32_t)strlen(text);
    // If not specified, use the lowest error correction level
    ErrorCorrectionLevel level = ECL_LOW;

    if (args.verbose) {
        printf("Generating QR:\n"
               "\ttext='%s'\n"
               "\ttextCharCount=%d\n"
               "\tlevel=%s\n",
               text,
               textCharCount,
               ErrorCorrectionLevelNames[level]);
    }


    // 1. Data analysis.
    // Infer encoding mode
    EncodingMode mode = qr_get_encoding_mode(text, textCharCount);
    // Infer QR version
    int32_t version = qr_get_version(mode, level, textCharCount);
    if (version == VERSION_INVALID) {
        fprintf(stderr, "Text too long, can't fit into any QR code version\n");
        exit(1);
    }

    // If possible, increase the error correction level while still staying in the same version
    for (ErrorCorrectionLevel newLevel = level + 1; newLevel <= ECL_HIGH; level = newLevel++) {
        if (textCharCount > MAX_CHAR_COUNT[mode][newLevel][version]) {
            break;
        }
    }

    if (args.verbose) {
        printf("Data analysis complete:\n"
           "\tmode=%s\n"
           "\tversion=%d\n"
           "\tlevel=%s\n",
           EncodingModeNames[mode],
           version + 1,
           ErrorCorrectionLevelNames[level]);
    }


    // 2. Data Encoding
    BitVec bv = {};

    // Encoding mode
    bv_append(&bv, (1U) << mode, 4);

    // Text length
    int32_t lengthBitCount = LENGTH_BITS_COUNT[mode][version];
    bv_append(&bv, textCharCount, lengthBitCount);

    // Text itself
    if (mode == EM_NUMERIC) {
        int32_t number = 0;
        for (int32_t i = 0; i < textCharCount; i++) {
            uint8_t ch = text[i];
            int32_t digit = ch - '0';
            number = number * 10 + digit;
            if (((i % 3) == 2) || (i == (textCharCount - 1))) {
                bv_append(&bv, number, (number >= 100) ? 10 : ((number >= 10) ? 7 : 4));
                number = 0;
            }
        }
    }
    else if (mode == EM_ALPHANUM) {
        int32_t number = 0;
        for (int32_t i = 0; i < textCharCount; i++) {
            uint8_t ch = text[i];

            int32_t addend = 0;
            if (ch <= '9') addend = ch - '0';
            else if (ch <= 'Z') addend = ch - 'A' + 10;
            else if (ch == ' ') addend = 36;
            else if (ch == '$') addend = 37;
            else if (ch == '%') addend = 38;
            else if (ch == '*') addend = 39;
            else if (ch == '+') addend = 40;
            else if (ch == '-') addend = 41;
            else if (ch == '.') addend = 42;
            else if (ch == '/') addend = 43;
            else if (ch == ':') addend = 44;
            else ASSERT(false); // unreachable

            number = number * 45 + addend;
            if (((i % 2) == 1) || (i == (textCharCount - 1))) {
                bv_append(&bv, number, (number >= 45) ? 11 : 6);
                number = 0;
            }
        }
    }
    else {
        for (int32_t i = 0; i < textCharCount; i++) {
            bv_append(&bv, text[i], 8);
        }
    }

    int32_t dataCodewordsCount = qr_calc_data_codewords_count(version, level);
    int32_t dataModulesCount = dataCodewordsCount * 8;

    // Terminator zeros
    int32_t terminatorLength = MIN(dataModulesCount - bv.size, 4);
    bv_append(&bv, 0, terminatorLength);

    // Zero padding for 8 bit alignment
    int32_t zeroPaddingLength = ((bv.size % 8) == 0) ? 0 : 8 - (bv.size % 8);
    bv_append(&bv, 0, zeroPaddingLength);

    // Padding pattern
    int32_t paddingLength = dataModulesCount - bv.size;
    for (int32_t i = 0; i < (paddingLength / 8); i++) {
        bv_append(&bv, (i & 1) ? 17 : 236, 8);
    }

    if (args.verbose) {
        printf("Data encoding complete:\n"
               "\tbits=");
        bv_print(stdout, bv);
    }


    // 3. Error correction coding
    uint8_t blocks[MAX_BLOCKS_COUNT][MAX_BLOCKS_LENGTH] = {};

    // Divide data codewords into blocks and calculate error correction codewords for them
    uint8_t *dataCodewords = bv.bytes;
    int32_t contentCodewordsCount = CONTENT_MODULES_COUNT[version] / 8;

    int32_t blocksCount = ERROR_CORRECTION_BLOCKS_COUNT[level][version];
    ASSERT(blocksCount <= MAX_BLOCKS_COUNT);

    int32_t shortBlocksCount = blocksCount - (contentCodewordsCount % blocksCount);
    int32_t shortBlockLength = contentCodewordsCount / blocksCount;
    ASSERT(shortBlockLength <= MAX_BLOCKS_LENGTH);

    int32_t errorCodewordsPerBlockCount = ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[level][version];
    const uint8_t *divisor = GENERATOR_POLYNOM[errorCodewordsPerBlockCount];

    int32_t dataCodewordsIndex = 0;
    for (int32_t i = 0; i < blocksCount; i++) {
        uint8_t *block = blocks[i];
        int32_t blockIndex = 0;

        // Copy data codewords to block
        int32_t currBlockLength = (i < shortBlocksCount) ? shortBlockLength : shortBlockLength + 1;
        int32_t dataCodewordsPerBlockCount = currBlockLength - errorCodewordsPerBlockCount;
        for (int32_t j = 0; j < dataCodewordsPerBlockCount; j++) {
            block[blockIndex++] = dataCodewords[dataCodewordsIndex++];
        }

        // Pad shorter blocks for easier interleaving later
        if (i < shortBlocksCount) {
            blockIndex++;
        }

        // Calculate error correcting codewords
        uint8_t errorCodewords[MAX_POLYNOM_DEGREE + 1] = {};
        int32_t remainderLength = gf256_polynom_divide(block,
                                                    currBlockLength,
                                                    divisor,
                                                    errorCodewordsPerBlockCount + 1,
                                                    errorCodewords);
        ASSERT(remainderLength == errorCodewordsPerBlockCount);

        if (args.verbose) {
            printf("   data[%3d]: ", currBlockLength);
            for (int32_t j = 0; j < currBlockLength; j++) {
                printf("%4d, ", block[j]);
            }
            printf("\n");
            printf("divisor[%3d]: ", errorCodewordsPerBlockCount + 1);
            for (int32_t j = 0; j < errorCodewordsPerBlockCount + 1; j++) {
                printf("%4d, ", divisor[j]);
            }
            printf("\n");
            printf(" result[%3d]: ", errorCodewordsPerBlockCount);
            for (int32_t j = 0; j < errorCodewordsPerBlockCount; j++) {
                printf("%4d, ", errorCodewords[j]);
            }
            printf("\n");
            printf("\n");
        }

        // Copy error correcting codewords to block
        int32_t errorCodewordsIndex = 0;
        for (int32_t j = 0; j < errorCodewordsPerBlockCount; j++) {
            block[blockIndex++] = errorCodewords[errorCodewordsIndex++];
        }
    }


    // 4. Codeword interleaving
    uint8_t interleavedCodewords[MAX_BLOCKS_COUNT * MAX_BLOCKS_LENGTH] = {};
    int32_t interleavedCodewordsCount = 0;
    for (int32_t i = 0; i < (shortBlockLength + 1); i++) {
        for (int32_t j = 0; j < blocksCount; j++) {
            if (j < shortBlocksCount && i == (shortBlockLength - errorCodewordsPerBlockCount)) {
                // skip the padding on shorter blocks
                continue;
            }
            interleavedCodewords[interleavedCodewordsCount++] = blocks[j][i];
        }
    }

    if (args.verbose) {
        printf("Codeword interleaving complete:\n"
               "\tinterleaved codewords=");
        for (int32_t i = 0; i < interleavedCodewordsCount; i++) {
            printf("%.2X ", interleavedCodewords[i]);
        }
        printf("\n\tinterleaved codeword count=%d\n", interleavedCodewordsCount);
    }


    // 5. Draw functional QR patterns
    QR qr = { .size = 4 * version + 21 };

    qr_draw_functional_patterns(&qr, version);

    if (args.verbose) {
        printf("Drawing functional patterns complete:\n");
        qr_print(stdout, &qr);
    }


    // 6. Reserve format & version modules
    qr_reserve_format_modules(&qr);
    qr_reserve_version_modules(&qr, version);

    if (args.verbose) {
        printf("Reserving format & version modules complete:\n");
        qr_print(stdout, &qr);
    }


    // 7. Draw QR data
    qr_draw_data(&qr, interleavedCodewords, interleavedCodewordsCount);

    if (args.verbose) {
        printf("Drawing QR data complete:\n");
        qr_print(stdout, &qr);
    }


    // 8. Apply data masking
    int32_t minPenalty = INT32_MAX;
    int32_t bestMask = 2;
    for (int32_t mask = 0; mask < MASK_COUNT; mask++) {
        qr_draw_format_modules(&qr, level, mask);
        qr_draw_version_modules(&qr, version);
        qr_apply_mask(&qr, mask);

        int32_t penalty = qr_calc_mask_penalty(&qr);
        if (penalty < minPenalty) {
            minPenalty = penalty;
            bestMask = mask;
        }

        // re-applying reverts the mask
        qr_apply_mask(&qr, mask);
    }
    qr_apply_mask(&qr, bestMask);

    if (args.verbose) {
        printf("Applying data mask %d complete:\n", bestMask);
        qr_print(stdout, &qr);
    }


    // 9. Draw QR format and version
    qr_draw_format_modules(&qr, level, bestMask);
    qr_draw_version_modules(&qr, version);

    if (args.verbose) {
        printf("Drawing format & version modules complete:\n");
        qr_print(stdout, &qr);
    }


    printf("\n");
    qr_print(stdout, &qr);
    printf("\n");

    return 0;
}
