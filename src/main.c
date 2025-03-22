#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qr.h"
#include "bv.h"
#include "rs.h"
#include "utils.h"

int32_t
main(int32_t argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <YOUR TEXT>\n", argv[0]);
        exit(1);
    }

#if BUILD_DEBUG
    bool debug = true;
#else
    bool debug = false;
#endif //BUILD_DEBUG

    char *text = argv[1];
    int32_t textCharCount = (int32_t)strlen(text);
    // If not specified, use the lowest error correction level
    ErrorCorrectionLevel level = Low;

    if (debug) {
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
    if (version == VersionInvalid) {
        fprintf(stderr, "Text too long, can't fit into any QR code version\n");
        exit(1);
    }

    // If possible, increase the error correction level while still staying in the same version
    for (ErrorCorrectionLevel newLevel = level + 1; newLevel <= High; level = newLevel++) {
        if (textCharCount > MaxCharCount[mode][newLevel][version]) {
            break;
        }
    }

    if (debug) {
        printf("Data analysis complete:\n"
           "\tmode=%s\n"
           "\tversion=%d\n"
           "\tlevel=%s\n",
           EncodingModeNames[mode],
           version + 1,
           ErrorCorrectionLevelNames[level]);
    }



    // 2. Data Encoding
    BitVec bv = {0};

    // Encoding mode
    bv_append(&bv, (1U) << mode, 4);

    // Text length
    int32_t lengthBitCount = LengthBitsCount[mode][version];
    bv_append(&bv, textCharCount, lengthBitCount);

    // Text itself
    if (mode == Numeric) {
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
    else if (mode == Alphanumeric) {
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
            else Assert(false); // unreachable

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
    int32_t terminatorLength = Min(dataModulesCount - bv.size, 4);
    bv_append(&bv, 0, terminatorLength);

    // Zero padding for 8 bit alignment
    int32_t zeroPaddingLength = ((bv.size % 8) == 0) ? 0 : 8 - (bv.size % 8);
    bv_append(&bv, 0, zeroPaddingLength);

    // Padding pattern
    int32_t paddingLength = dataModulesCount - bv.size;
    for (int32_t i = 0; i < (paddingLength / 8); i++) {
        bv_append(&bv, (i & 1) ? 17 : 236, 8);
    }

    if (debug) {
        printf("Data encoding complete:\n"
               "\tbits=");
        bv_print(bv);
    }


    // 3. Error correction coding
    uint8_t blocks[MaxBlocksCount][MaxBlocksLength] = {0};

    // Divide data codewords into blocks and calculate error correction codewords for them
    uint8_t *dataCodewords = bv.bytes;
    int32_t contentCodewordsCount = ContentModulesCount[version] / 8;

    int32_t blocksCount = ErrorCorrectionBlocksCount[level][version];
    Assert(blocksCount <= MaxBlocksCount);

    int32_t shortBlocksCount = blocksCount - (contentCodewordsCount % blocksCount);
    int32_t shortBlockLength = contentCodewordsCount / blocksCount;
    Assert(shortBlockLength <= MaxBlocksLength);

    int32_t errorCodewordsPerBlockCount = ErrorCorrectionCodewordsPerBlockCount[level][version];

    int32_t dataCodewordsIndex = 0;
    for (int32_t i = 0; i < blocksCount; i++) {
        uint8_t *block = blocks[i];
        int32_t blockIndex = 0;

        // Copy data codewords to block
        int32_t dataCodewordsPerBlockCount = (i < shortBlocksCount) ? shortBlockLength - errorCodewordsPerBlockCount
                                                                : shortBlockLength - errorCodewordsPerBlockCount + 1;
        for (int32_t j = 0; j < dataCodewordsPerBlockCount; j++) {
            block[blockIndex++] = dataCodewords[dataCodewordsIndex++];
        }

        // Pad shorter blocks for easier interleaving later
        if (i < shortBlocksCount) {
            blockIndex++;
        }

        // Calculate error correcting codewords
        uint8_t errorCodewords[MaxPolygonDegree + 1] = {0};
        gf256_poly_divide(block,
                          (i < shortBlocksCount) ? shortBlockLength - 1 : shortBlockLength,
                          GenPoly[errorCodewordsPerBlockCount],
                          errorCodewordsPerBlockCount,
                          errorCodewords);

        // Copy error correcting codewords to block
        int32_t errorCodewordsIndex = 0;
        for (int32_t j = 0; j < errorCodewordsPerBlockCount; j++) {
            block[blockIndex++] = errorCodewords[errorCodewordsIndex++];
        }
    }

    // 4. Codeword interleaving
    uint8_t interleavedCodewords[MaxBlocksCount * MaxBlocksLength] = {0};
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

    if (debug) {
        printf("Codeword interleaving complete:\n"
               "\tinterleaved codewords=");
        for (int32_t i = 0; i < interleavedCodewordsCount; i++) {
            printf("%.2X ", interleavedCodewords[i]);
        }
        printf("\n\tinterleaved codeword count=%d\n", interleavedCodewordsCount);
    }


    // 5. Draw functional QR patterns
    uint8_t qrCode[177 * 177] = {0};
    int32_t qrSize = 4 * version + 21;

    qr_draw_finder_patterns(qrCode, qrSize);
    qr_draw_alignment_patterns(qrCode, qrSize, version);
    qr_draw_timing_patterns(qrCode, qrSize);
    qr_draw_module(qrCode, qrSize, 4 * version + 13, 8, ModuleFunctional | ModuleBlack); // dark module

    if (debug) {
        printf("Drawing functional patterns complete:\n");
        qr_print(qrCode, qrSize);
    }


    // 6. Reserve format & version modules
    // Vertical format modules
    qr_draw_rectangle(qrCode, qrSize, 0, 8, 1, 6, ModuleReserved);
    qr_draw_rectangle(qrCode, qrSize, 7, 8, 1, 2, ModuleReserved);
    qr_draw_rectangle(qrCode, qrSize, qrSize - 7, 8, 1, 8, ModuleReserved);
    // Horizontal format modules
    qr_draw_rectangle(qrCode, qrSize, 8, 0, 6, 1, ModuleReserved);
    qr_draw_rectangle(qrCode, qrSize, 8, 7, 2, 1, ModuleReserved);
    qr_draw_rectangle(qrCode, qrSize, 8, qrSize - 8, 8, 1, ModuleReserved);

    if (version > 5) {
        // Reserve version modules
        qr_draw_rectangle(qrCode, qrSize, 0, qrSize - 11, 3, 6, ModuleReserved);
        qr_draw_rectangle(qrCode, qrSize, qrSize - 11, 0, 6, 3, ModuleReserved);
    }

    if (debug) {
        printf("Reserving format & version modules complete:\n");
        qr_print(qrCode, qrSize);
    }


    // 7. Draw QR data
    qr_draw_data(qrCode, qrSize, interleavedCodewords, interleavedCodewordsCount);

    if (debug) {
        printf("Drawing QR data complete:\n");
        qr_print(qrCode, qrSize);
    }


    // 8. Apply data masking
    int32_t mask = 0; //TODO calc best mask
    qr_apply_mask(qrCode, qrSize, mask);

    if (debug) {
        printf("Applying data masking complete:\n");
        qr_print(qrCode, qrSize);
    }


    // 9. Draw QR format and version
    qr_draw_format_bits(qrCode, qrSize, level, mask);

    if (debug) {
        printf("Drawing format & version modules complete:\n");
        qr_print(qrCode, qrSize);
    }

    printf("\n");
    qr_print(qrCode, qrSize);
    printf("\n");

    return 0;
}
