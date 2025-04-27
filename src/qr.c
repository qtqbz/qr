#include <stdio.h>
#include <string.h>

#include "qr.h"
#include "utils.h"

int32_t
qr_calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level)
{
    int32_t contentCodewordsCount = ContentModulesCount[version] / 8;
    int32_t errorCorrectionCodewordsCount = ErrorCorrectionBlocksCount[level][version] * ErrorCorrectionCodewordsPerBlockCount[level][version];
    int32_t dataCodewordsCount = contentCodewordsCount - errorCorrectionCodewordsCount;
    return dataCodewordsCount;
}

int32_t
qr_find_first_unmatch_index(const char *text, int32_t textCharCount, char *charsToMatch, int32_t offset)
{
    for (int32_t i = offset; i < textCharCount; i++) {
        char ch = text[i];
        if (!strchr(charsToMatch, ch)) {
            return i;
        }
    }
    return -1;
}

EncodingMode
qr_get_encoding_mode(const char *text, int32_t textCharCount)
{
    int32_t i = qr_find_first_unmatch_index(text, textCharCount, "0123456789", 0);
    if (i < 0) {
        return Numeric;
    }
    i = qr_find_first_unmatch_index(text, textCharCount, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:", i + 1);
    if (i < 0) {
        return Alphanumeric;
    }
    return Byte;
}

int32_t
qr_get_version(EncodingMode mode, ErrorCorrectionLevel level, int32_t textCharCount)
{
    for (int32_t version = MinVersion; version <= MaxVersion; version++) {
        if (textCharCount <= MaxCharCount[mode][level][version]) {
            return version;
        }
    }
    return VersionInvalid;
}

void
qr_draw_module(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, ModuleValue value)
{
    if ((0 <= row && row < qrSize) && (0 <= column && column < qrSize)) {
        qrCode[row * qrSize + column] = value;
    }
}

void
qr_draw_rectangle(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, int32_t width, int32_t height, ModuleValue value)
{
    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            qr_draw_module(qrCode, qrSize, row + i, column + j, value);
        }
    }
}

void
qr_draw_square(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, int32_t size, ModuleValue value)
{
    qr_draw_rectangle(qrCode, qrSize, row, column, size, size, value);
}

void
qr_draw_finder_pattern(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column)
{
    qr_draw_square(qrCode, qrSize, row - 1, column - 1, 9, FunctionalWhite); // separator
    qr_draw_square(qrCode, qrSize, row + 0, column + 0, 7, FunctionalBlack);
    qr_draw_square(qrCode, qrSize, row + 1, column + 1, 5, FunctionalWhite);
    qr_draw_square(qrCode, qrSize, row + 2, column + 2, 3, FunctionalBlack);
}

void
qr_draw_finder_patterns(uint8_t *qrCode, int32_t qrSize)
{
    qr_draw_finder_pattern(qrCode, qrSize, 0, 0);
    qr_draw_finder_pattern(qrCode, qrSize, 0, qrSize - 7);
    qr_draw_finder_pattern(qrCode, qrSize, qrSize - 7, 0);
}

void
qr_draw_alignment_pattern(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column)
{
    qr_draw_square(qrCode, qrSize, row + 0, column + 0, 5, FunctionalBlack);
    qr_draw_square(qrCode, qrSize, row + 1, column + 1, 3, FunctionalWhite);
    qr_draw_module(qrCode, qrSize, row + 2, column + 2, FunctionalBlack);
}

void
qr_draw_alignment_patterns(uint8_t *qrCode, int32_t qrSize, int32_t version)
{
    for (int32_t i = 0; i < AlignmentCoordinatesCount; i++) {
        int32_t rowCenter = AlignmentCoordinates[version][i];
        if (rowCenter == 0) {
            break;
        }
        for (int32_t j = 0; j < AlignmentCoordinatesCount; j++) {
            int32_t columnCenter = AlignmentCoordinates[version][j];
            if (columnCenter == 0) {
                break;
            }
            if (qrCode[(rowCenter - 2) * qrSize + (columnCenter - 2)] == 0
                && qrCode[(rowCenter - 2) * qrSize + (columnCenter + 2)] == 0
                && qrCode[(rowCenter + 2) * qrSize + (columnCenter - 2)] == 0
                && qrCode[(rowCenter + 2) * qrSize + (columnCenter + 2)] == 0) {
                int32_t row = rowCenter - 2;
                int32_t column = columnCenter - 2;
                qr_draw_alignment_pattern(qrCode, qrSize, row, column);
            }
        }
    }
}

void
qr_draw_timing_patterns(uint8_t *qrCode, int32_t qrSize)
{
    ModuleValue value = FunctionalBlack;
    for (int32_t i = 8; i < (qrSize - 7); i++) {
        qr_draw_module(qrCode, qrSize, 6, i, value);
        qr_draw_module(qrCode, qrSize, i, 6, value);
        value = value ^ FunctionalBlack ^ FunctionalWhite;
    }
}

void
qr_draw_data(uint8_t *qrCode, int32_t qrSize, uint8_t *codewords, int32_t codewordsCount)
{
    int32_t column = qrSize - 1;
    int32_t row = qrSize - 1;
    int32_t rowDirection = -1; // -1 == up, +1 == down
    int32_t codewordsIndex = 0;
    int32_t bitIndex = 7;

    while (row >= 0 && column >= 0) {
        if (qrCode[row * qrSize + column] == 0) {
            if (codewordsIndex < codewordsCount) {
                uint8_t codeword = codewords[codewordsIndex];
                qrCode[row * qrSize + column] = ((codeword >> bitIndex) & 1) ? DataBlack : DataWhite;
                if (bitIndex == 0) {
                    codewordsIndex++;
                    bitIndex = 7;
                }
                else {
                    bitIndex--;
                }
            }
            else {
                qrCode[row * qrSize + column] = DataWhite;
            }
        } else {
            if ((column == 6) || (column > 6 && column % 2 == 0) || (column < 6 && column % 2 == 1)) {
                column -= 1;
            }
            else {
                column += 1;
                row += rowDirection;
                if (row < 0 || row == qrSize) {
                    rowDirection = -rowDirection;
                    column -= 2;
                    row += rowDirection;
                }
            }
        }
    }
}

void
qr_print(FILE *out, uint8_t *qrCode, int32_t qrSize)
{
    for (int32_t i = -1; i <= qrSize; i++) {
        for (int32_t j = -1; j <= qrSize; j++) {
            if (i < 0 || i == qrSize || j < 0 || j == qrSize) {
                // Drawing frame
                fprintf(out, "\033[47m  \033[0m");
                continue;
            }

            uint8_t value = qrCode[i * qrSize + j];
            if (value == DataBlack || value == FunctionalBlack) {
                fprintf(out, "\033[40m  \033[0m");
            }
            else if (value == DataWhite || value == FunctionalWhite) {
                fprintf(out, "\033[47m  \033[0m");
            }
            else {
                // Empty module
                fprintf(out, "\033[50m  \033[0m");
            }
        }
        fprintf(out, "\n");
    }
}

void
qr_apply_mask(uint8_t *qrCode, int32_t qrSize, int32_t mask)
{
    for (int32_t row = 0; row < qrSize; row++) {
        for (int32_t column = 0; column < qrSize; column++) {
            uint8_t value = qrCode[row * qrSize + column];
            if ((value != DataBlack) && (value != DataWhite)) {
                continue;
            }

            bool invert = false;
            switch (mask) {
                case 0: invert = (row + column) % 2 == 0; break;
                case 1: invert = column % 2 == 0; break;
                case 2: invert = row % 3 == 0; break;
                case 3: invert = (row + column) % 3 == 0; break;
                case 4: invert = (row / 3 + column / 2) % 2 == 0; break;
                case 5: invert = row * column % 2 + row * column % 3 == 0; break;
                case 6: invert = (row * column % 2 + row * column % 3) % 2 == 0; break;
                case 7: invert = ((row + column) % 2 + row * column % 3) % 2 == 0; break;
                default: Assert(false); // unreachable
            }
            if (invert) {
                qrCode[row * qrSize + column] = value ^ DataBlack ^ DataWhite;
            }
        }
    }
}

void
qr_draw_format_bits(uint8_t *qrCode, int32_t qrSize, ErrorCorrectionLevel level, int32_t mask)
{
    uint32_t formatBits = FormatBits[level][mask];
    int32_t bitIndex = 0;
    for (int32_t row = 0; row < qrSize; row++) {
        if (row == 6) {
            row++; // skip timing pattern module
        }
        else if (row == 9) {
            row = qrSize - 7; // skip to the opposite side
        }
        qr_draw_module(qrCode, qrSize, row, 8, ((formatBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite);
        bitIndex++;
    }

    bitIndex = 14;
    for (int32_t column = 0; column < qrSize; column++) {
        if (column == 6) {
            column++; // skip timing pattern module
        }
        else if (column == 9) {
            column = qrSize - 8; // skip to the opposite side
            bitIndex++;
        }
        qr_draw_module(qrCode, qrSize, 8, column, ((formatBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite);
        bitIndex--;
    }
}

void
qr_draw_version_bits(uint8_t *qrCode, int32_t qrSize, int32_t version)
{
    Assert(version > 5 && version <= MaxVersion);

    uint32_t versionBits = VersionBits[version];
    int32_t bitIndex = 0;

    for (int32_t i = 0; i < 6; i++) {
        for (int32_t j = 0; j < 3; j++) {
            ModuleValue value = ((versionBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite;
            qr_draw_module(qrCode, qrSize, i, qrSize - 11 + j, value);
            qr_draw_module(qrCode, qrSize, qrSize - 11 + j, i, value);
            bitIndex++;
        }
    }
}
