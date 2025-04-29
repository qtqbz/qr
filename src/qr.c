#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qr.h"
#include "utils.h"

#define QR_MODULE_VALUE(qr, row, column) ((qr)->modules[(row) * (qr)->size + (column)])
#define QR_MODULE_COLOR(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).color)
#define QR_MODULE_TYPE(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).type)

static const ModuleValue DataWhite = { .type = TypeData, .color = ColorWhite };
static const ModuleValue DataBlack = { .type = TypeData, .color = ColorBlack };
static const ModuleValue FunctionalWhite = { .type = TypeFunctional, .color = ColorWhite };
static const ModuleValue FunctionalBlack = { .type = TypeFunctional, .color = ColorBlack };

int32_t
qr_calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level)
{
    int32_t contentCodewordsCount = ContentModulesCount[version] / 8;
    int32_t errorCorrectionCodewordsCount = ErrorCorrectionBlocksCount[level][version] * ErrorCorrectionCodewordsPerBlockCount[level][version];
    int32_t dataCodewordsCount = contentCodewordsCount - errorCorrectionCodewordsCount;
    return dataCodewordsCount;
}

static int32_t
find_first_unmatch_index(const char *text, int32_t textCharCount, char *charsToMatch, int32_t offset)
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
    int32_t i = find_first_unmatch_index(text, textCharCount, "0123456789", 0);
    if (i < 0) {
        return Numeric;
    }
    i = find_first_unmatch_index(text, textCharCount, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:", i + 1);
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

static void
draw_module(QR *qr, int32_t row, int32_t column, ModuleValue value)
{
    if ((0 <= row && row < qr->size) && (0 <= column && column < qr->size)) {
        QR_MODULE_VALUE(qr, row, column) = value;
    }
}

static void
draw_rectangle(QR *qr, int32_t row, int32_t column, int32_t width, int32_t height, ModuleValue value)
{
    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            draw_module(qr, row + i, column + j, value);
        }
    }
}

static void
draw_square(QR *qr, int32_t row, int32_t column, int32_t size, ModuleValue value)
{
    draw_rectangle(qr, row, column, size, size, value);
}

static void
draw_finder_pattern(QR *qr, int32_t row, int32_t column)
{
    draw_square(qr, row - 1, column - 1, 9, FunctionalWhite); // separator
    draw_square(qr, row + 0, column + 0, 7, FunctionalBlack);
    draw_square(qr, row + 1, column + 1, 5, FunctionalWhite);
    draw_square(qr, row + 2, column + 2, 3, FunctionalBlack);
}

static void
draw_finder_patterns(QR *qr)
{
    draw_finder_pattern(qr, 0, 0);
    draw_finder_pattern(qr, 0, qr->size - 7);
    draw_finder_pattern(qr, qr->size - 7, 0);
}

static void
draw_alignment_pattern(QR *qr, int32_t row, int32_t column)
{
    draw_square(qr, row + 0, column + 0, 5, FunctionalBlack);
    draw_square(qr, row + 1, column + 1, 3, FunctionalWhite);
    draw_module(qr, row + 2, column + 2, FunctionalBlack);
}

static void
draw_alignment_patterns(QR *qr, int32_t version)
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
            if (QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter - 2) == TypeNone
                && QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter + 2) == TypeNone
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter - 2) == TypeNone
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter + 2) == TypeNone) {
                int32_t row = rowCenter - 2;
                int32_t column = columnCenter - 2;
                draw_alignment_pattern(qr, row, column);
            }
        }
    }
}

static void
draw_timing_patterns(QR *qr)
{
    ModuleValue value = FunctionalBlack;
    for (int32_t i = 8; i < (qr->size - 7); i++) {
        draw_module(qr, 6, i, value);
        draw_module(qr, i, 6, value);
        value.color = (value.color == ColorWhite) ? ColorBlack : ColorWhite;
    }
}

void
qr_draw_functional_patterns(QR *qr, int32_t version)
{
    draw_finder_patterns(qr);
    draw_alignment_patterns(qr, version);
    draw_timing_patterns(qr);
    draw_module(qr, 4 * version + 13, 8, FunctionalBlack); // dark module
}

void
qr_reserve_format_modules(QR *qr)
{
    // Vertical format modules
    draw_rectangle(qr, 0, 8, 1, 6, FunctionalBlack);
    draw_rectangle(qr, 7, 8, 1, 2, FunctionalBlack);
    draw_rectangle(qr, qr->size - 7, 8, 1, 8, FunctionalBlack);

    // Horizontal format modules
    draw_rectangle(qr, 8, 0, 6, 1, FunctionalBlack);
    draw_rectangle(qr, 8, 7, 2, 1, FunctionalBlack);
    draw_rectangle(qr, 8, qr->size - 8, 8, 1, FunctionalBlack);
}

void
qr_reserve_version_modules(QR *qr, int32_t version)
{
    if (version < 6) {
        return;
    }
    // Vertical version modules
    draw_rectangle(qr, qr->size - 11, 0, 6, 3, FunctionalBlack);

    // Horizontal version modules
    draw_rectangle(qr, 0, qr->size - 11, 3, 6, FunctionalBlack);
}

void
qr_draw_data(QR *qr, uint8_t *codewords, int32_t codewordsCount)
{
    int32_t column = qr->size - 1;
    int32_t row = qr->size - 1;
    int32_t rowDirection = -1; // -1 == up, +1 == down
    int32_t codewordsIndex = 0;
    int32_t bitIndex = 7;

    while (row >= 0 && column >= 0) {
        if (QR_MODULE_TYPE(qr, row, column) == TypeNone) {
            if (codewordsIndex < codewordsCount) {
                uint8_t codeword = codewords[codewordsIndex];
                QR_MODULE_VALUE(qr, row, column) = ((codeword >> bitIndex) & 1) ? DataBlack : DataWhite;
                if (bitIndex == 0) {
                    codewordsIndex++;
                    bitIndex = 7;
                }
                else {
                    bitIndex--;
                }
            }
            else {
                QR_MODULE_VALUE(qr, row, column) = DataWhite;
            }
        } else {
            if ((column == 6) || (column > 6 && column % 2 == 0) || (column < 6 && column % 2 == 1)) {
                column -= 1;
            }
            else {
                column += 1;
                row += rowDirection;
                if (row < 0 || row == qr->size) {
                    rowDirection = -rowDirection;
                    column -= 2;
                    row += rowDirection;
                }
            }
        }
    }
}

void
qr_print(FILE *out, QR *qr)
{
    for (int32_t i = -4; i <= qr->size + 4; i++) {
        for (int32_t j = -4; j <= qr->size + 4; j++) {
            if (i < 0 || i >= qr->size || j < 0 || j >= qr->size) {
                // Drawing frame
                fprintf(out, "\033[47m  \033[0m");
                continue;
            }

            if (QR_MODULE_COLOR(qr, i, j) == ColorWhite) {
                fprintf(out, "\033[47m  \033[0m");
            }
            else {
                fprintf(out, "\033[40m  \033[0m");
            }
        }
        fprintf(out, "\n");
    }
}

void
qr_apply_mask(QR *qr, int32_t mask)
{
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size; column++) {
            ModuleValue value = QR_MODULE_VALUE(qr, row, column);
            if (value.type != TypeData) {
                continue;
            }

            bool invert = false;
            switch (mask) {
                case 0: invert = ((row + column) % 2) == 0; break;
                case 1: invert = (row % 2) == 0; break;
                case 2: invert = (column % 3) == 0; break;
                case 3: invert = ((row + column) % 3) == 0; break;
                case 4: invert = ((row / 2 + column / 3) % 2) == 0; break;
                case 5: invert = ((row * column) % 2) + ((row * column) % 3) == 0; break;
                case 6: invert = (((row * column) % 2) + ((row * column) % 3) % 2) == 0; break;
                case 7: invert = (((row + column) % 2) + ((row * column) % 3) % 2) == 0; break;
                default: Assert(false); // unreachable
            }
            if (invert) {
                QR_MODULE_COLOR(qr, row, column) = (value.color == ColorWhite) ? ColorBlack : ColorWhite;
            }
        }
    }
}

static int32_t
calc_rule1_penalty(QR *qr)
{
    int32_t penalty = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        int32_t count = 1;
        ModuleColor prevColor = QR_MODULE_COLOR(qr, row, 0);
        for (int32_t column = 1; column < qr->size; column++) {
            ModuleColor color = QR_MODULE_COLOR(qr, row, column);
            if (color == prevColor) {
                count++;
                if (count == 5) {
                    penalty += 3;
                } else if (count > 5) {
                    penalty++;
                }
            } else {
                prevColor = color;
                count = 1;
            }
        }
    }
    for (int32_t column = 0; column < qr->size; column++) {
        int32_t count = 1;
        ModuleColor prevColor = QR_MODULE_COLOR(qr, 0, column);
        for (int32_t row = 1; row < qr->size; row++) {
            ModuleColor color = QR_MODULE_COLOR(qr, row, column);
            if (color == prevColor) {
                count++;
                if (count == 5) {
                    penalty += 3;
                } else if (count > 5) {
                    penalty++;
                }
            } else {
                prevColor = color;
                count = 1;
            }
        }
    }
    return penalty;
}

static int32_t
calc_rule2_penalty(QR *qr)
{
    int32_t penalty = 0;
    for (int32_t row = 0; row < qr->size - 1; row++) {
        for (int32_t column = 0; column < qr->size - 1; column++) {
            ModuleColor color1 = QR_MODULE_COLOR(qr, row + 0, column + 0);
            ModuleColor color2 = QR_MODULE_COLOR(qr, row + 0, column + 1);
            ModuleColor color3 = QR_MODULE_COLOR(qr, row + 1, column + 0);
            ModuleColor color4 = QR_MODULE_COLOR(qr, row + 1, column + 1);
            if ((color1 == color2) && (color2 == color3) && (color3 == color4)) {
                penalty += 3;
            }
        }
    }
    return penalty;
}

static int32_t
calc_rule3_penalty(QR *qr)
{
    int32_t penalty = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size - 7; column++) {
            if ((QR_MODULE_COLOR(qr, row, column + 0) == 1)
                && (QR_MODULE_COLOR(qr, row, column + 1) == 0)
                && (QR_MODULE_COLOR(qr, row, column + 2) == 1)
                && (QR_MODULE_COLOR(qr, row, column + 3) == 1)
                && (QR_MODULE_COLOR(qr, row, column + 4) == 1)
                && (QR_MODULE_COLOR(qr, row, column + 5) == 0)
                && (QR_MODULE_COLOR(qr, row, column + 6) == 1)) {
                uint32_t preWhiteModuleCount = 0;
                uint32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((column - i) < 0) || (QR_MODULE_COLOR(qr, row, column - i) == ColorWhite)) {
                        preWhiteModuleCount++;
                    }
                    if (((column + 6 + i) >= qr->size) || (QR_MODULE_COLOR(qr, row, column + 6 + i) == ColorWhite)) {
                        postWhiteModuleCount++;
                    }
                }
                if ((preWhiteModuleCount == 4) || (postWhiteModuleCount == 4)) {
                    penalty += 40;
                }
            }
        }
    }
    for (int32_t column = 0; column < qr->size; column++) {
        for (int32_t row = 0; row < qr->size - 7; row++) {
            if ((QR_MODULE_COLOR(qr, row + 0, column) == 1)
                && (QR_MODULE_COLOR(qr, row + 1, column) == 0)
                && (QR_MODULE_COLOR(qr, row + 2, column) == 1)
                && (QR_MODULE_COLOR(qr, row + 3, column) == 1)
                && (QR_MODULE_COLOR(qr, row + 4, column) == 1)
                && (QR_MODULE_COLOR(qr, row + 5, column) == 0)
                && (QR_MODULE_COLOR(qr, row + 6, column) == 1)) {
                uint32_t preWhiteModuleCount = 0;
                uint32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((row - i) < 0) || (QR_MODULE_COLOR(qr, row - i, column) == ColorWhite)) {
                        preWhiteModuleCount++;
                    }
                    if (((row + 6 + i) >= qr->size) || (QR_MODULE_COLOR(qr, row + 6 + i, column) == ColorWhite)) {
                        postWhiteModuleCount++;
                    }
                }
                if ((preWhiteModuleCount == 4) || (postWhiteModuleCount == 4)) {
                    penalty += 40;
                }
            }
        }
    }
    return penalty;
}

static int32_t
calc_rule4_penalty(QR *qr)
{
    int32_t totalModuleCount = qr->size * qr->size;
    int32_t blackModuleCount = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size - 11; column++) {
            if (QR_MODULE_COLOR(qr, row, column) == ColorBlack) {
                blackModuleCount++;
            }
        }
    }
    int32_t fivePercentVariances = abs(blackModuleCount * 2 - totalModuleCount) * 10 / totalModuleCount;
    int32_t penalty = fivePercentVariances * 10;
    return penalty;
}

int32_t
qr_calc_mask_penalty(QR *qr)
{
    int32_t penalty1 = calc_rule1_penalty(qr);
    int32_t penalty2 = calc_rule2_penalty(qr);
    int32_t penalty3 = calc_rule3_penalty(qr);
    int32_t penalty4 = calc_rule4_penalty(qr);
    return penalty1 + penalty2 + penalty3 + penalty4;
}

void
qr_draw_format_modules(QR *qr, ErrorCorrectionLevel level, int32_t mask)
{
    uint32_t formatBits = FormatBits[level][mask];
    int32_t bitIndex = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        if (row == 6) {
            row++; // skip timing pattern module
        }
        else if (row == 9) {
            row = qr->size - 7; // skip to the opposite side
        }
        draw_module(qr, row, 8, ((formatBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite);
        bitIndex++;
    }

    bitIndex = 14;
    for (int32_t column = 0; column < qr->size; column++) {
        if (column == 6) {
            column++; // skip timing pattern module
        }
        else if (column == 9) {
            column = qr->size - 8; // skip to the opposite side
            bitIndex++;
        }
        draw_module(qr, 8, column, ((formatBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite);
        bitIndex--;
    }
}

void
qr_draw_version_modules(QR *qr, int32_t version)
{
    Assert(MinVersion <= version && version <= MaxVersion);

    if (version < 6) {
        return;
    }
    uint32_t versionBits = VersionBits[version];
    int32_t bitIndex = 0;

    for (int32_t i = 0; i < 6; i++) {
        for (int32_t j = 0; j < 3; j++) {
            ModuleValue value = ((versionBits >> bitIndex) & 1) ? FunctionalBlack : FunctionalWhite;
            draw_module(qr, i, qr->size - 11 + j, value);
            draw_module(qr, qr->size - 11 + j, i, value);
            bitIndex++;
        }
    }
}
