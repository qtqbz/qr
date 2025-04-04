#ifndef QR_H
#define QR_H

#include <stdint.h>

#define MinVersion 0
#define MaxVersion 39
#define VersionCount 40
#define VersionInvalid (-1)

#define MaxBlocksCount 81
#define MaxBlocksLength 153

#define AlignmentCoordinatesCount 7

#define MaskCount 8

typedef uint32_t EncodingMode;
enum EncodingMode
{
    Numeric,
    Alphanumeric,
    Byte,
    //Kanji,

    EncodingModeCount,
};

static const char *const EncodingModeNames[EncodingModeCount] = {
    "Numeric (0001)",
    "Alphanumeric (0010)",
    "Byte (0100)",
    //"Kanji (1000)",
};

typedef uint32_t ErrorCorrectionLevel;
enum ErrorCorrectionLevel
{
    Low,      // ~7%
    Medium,   // ~15%
    Quartile, // ~25%
    High,     // ~30%

    ErrorCorrectionLevelCount,
};

static const char *const ErrorCorrectionLevelNames[ErrorCorrectionLevelCount] = {
    "Low",
    "Medium",
    "Quartile",
    "High",
};

typedef uint8_t ModuleFlag;
enum ModuleFlag
{
    ModuleFunctional = 0b0001,
    ModuleReserved   = 0b0010,
    ModuleWhite      = 0b0100,
    ModuleBlack      = 0b1000,
};

// Maximum length of the encoded text.
static const int32_t MaxCharCount[EncodingModeCount][ErrorCorrectionLevelCount][VersionCount] = {
    // Numeric
    {
        {  41,   77,  127,  187,  255,  322,  370,  461,  552,  652,
          772,  883, 1022, 1101, 1250, 1408, 1548, 1725, 1903, 2061,
         2232, 2409, 2620, 2812, 3057, 3283, 3517, 3669, 3909, 4158,
         4417, 4686, 4965, 5253, 5529, 5836, 6153, 6479, 6743, 7089, },
        {  34,   63,  101,  149,  202,  255,  293,  365,  432,  513,
          604,  691,  796,  871,  991, 1082, 1212, 1346, 1500, 1600,
         1708, 1872, 2059, 2188, 2395, 2544, 2701, 2857, 3035, 3289,
         3486, 3693, 3909, 4134, 4343, 4588, 4775, 5039, 5313, 5596, },
        {  27,   48,   77,  111,  144,  178,  207,  259,  312,  364,
          427,  489,  580,  621,  703,  775,  876,  948, 1063, 1159,
         1224, 1358, 1468, 1588, 1718, 1804, 1933, 2085, 2181, 2358,
         2473, 2670, 2805, 2949, 3081, 3244, 3417, 3599, 3791, 3993, },
        {  17,   34,   58,   82,  106,  139,  154,  202,  235,  288,
          331,  374,  427,  468,  530,  602,  674,  746,  813,  919,
          969, 1056, 1108, 1228, 1286, 1425, 1501, 1581, 1677, 1782,
         1897, 2022, 2157, 2301, 2361, 2524, 2625, 2735, 2927, 3057, }
    },
    // Alphanumeric
    {
        {  25,   47,   77,  114,  154,  195,  224,  279,  335,  395,
          468,  535,  619,  667,  758,  854,  938, 1046, 1153, 1249,
         1352, 1460, 1588, 1704, 1853, 1990, 2132, 2223, 2369, 2520,
         2677, 2840, 3009, 3183, 3351, 3537, 3729, 3927, 4087, 4296, },
        {  20,   38,   61,   90,  122,  154,  178,  221,  262,  311,
          366,  419,  483,  528,  600,  656,  734,  816,  909,  970,
         1035, 1134, 1248, 1326, 1451, 1542, 1637, 1732, 1839, 1994,
         2113, 2238, 2369, 2506, 2632, 2780, 2894, 3054, 3220, 3391, },
        {  16,   29,   47,   67,   87,  108,  125,  157,  189,  221,
          259,  296,  352,  376,  426,  470,  531,  574,  644,  702,
          742,  823,  890,  963, 1041, 1094, 1172, 1263, 1322, 1429,
         1499, 1618, 1700, 1787, 1867, 1966, 2071, 2181, 2298, 2420, },
        {  10,   20,   35,   50,   64,   84,   93,  122,  143,  174,
          200,  227,  259,  283,  321,  365,  408,  452,  493,  557,
          587,  640,  672,  744,  779,  864,  910,  958, 1016, 1080,
         1150, 1226, 1307, 1394, 1431, 1530, 1591, 1658, 1774, 1852, }
    },
    // Byte
    {
        {  17,   32,   53,   78,  106,  134,  154,  192,  230,  271,
          321,  367,  425,  458,  520,  586,  644,  718,  792,  858,
          929, 1003, 1091, 1171, 1273, 1367, 1465, 1528, 1628, 1732,
         1840, 1952, 2068, 2188, 2303, 2431, 2563, 2699, 2809, 2953, },
        {  14,   26,   42,   62,   84,  106,  122,  152,  180,  213,
          251,  287,  331,  362,  412,  450,  504,  560,  624,  666,
          711,  779,  857,  911,  997, 1059, 1125, 1190, 1264, 1370,
         1452, 1538, 1628, 1722, 1809, 1911, 1989, 2099, 2213, 2331, },
        {  11,   20,   32,   46,   60,   74,   86,  108,  130,  151,
          177,  203,  241,  258,  292,  322,  364,  394,  442,  482,
          509,  565,  611,  661,  715,  751,  805,  868,  908,  982,
         1030, 1112, 1168, 1228, 1283, 1351, 1423, 1499, 1579, 1663, },
        {   7,   14,   24,   34,   44,   58,   64,   84,   98,  119,
          137,  155,  177,  194,  220,  250,  280,  310,  338,  382,
          403,  439,  461,  511,  535,  593,  625,  658,  698,  742,
          790,  842,  898,  958,  983, 1051, 1093, 1139, 1219, 1273, }
    },
};

// Number of bits allocated for the length of the encoded text.
static const int32_t LengthBitsCount[EncodingModeCount][VersionCount] = {
    // Numeric
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 12,
      12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
      12, 12, 12, 12, 12, 12, 14, 14, 14, 14,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, },
    // Alphanumeric
    {  9,  9,  9,  9,  9,  9,  9,  9,  9, 11,
      11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
      11, 11, 11, 11, 11, 11, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13, },
    // Byte
    {  8,  8,  8,  8,  8,  8,  8,  8,  8, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, },
};

// Number of data modules and error correction modules.
static const int32_t ContentModulesCount[VersionCount] = {
      208,   359,   567,   807,  1079,  1383,  1568,  1936,  2336,  2768,
     3232,  3728,  4256,  4651,  5243,  5867,  6523,  7211,  7931,  8683,
     9252, 10068, 10916, 11796, 12708, 13652, 14628, 15371, 16411, 17483,
    18587, 19723, 20891, 22091, 23008, 24272, 25568, 26896, 28256, 29648,
};

// Number of ECC blocks.
static const int32_t ErrorCorrectionBlocksCount[ErrorCorrectionLevelCount][VersionCount] = {
    // Low
    {  1,  1,  1,  1,  1,  2,  2,  2,  2,  4,
       4,  4,  4,  4,  6,  6,  6,  6,  7,  8,
       8,  9,  9, 10, 12, 12, 12, 13, 14, 15,
      16, 17, 18, 19, 19, 20, 21, 22, 24, 25, },
    // Medium
    {  1,  1,  1,  2,  2,  4,  4,  4,  5,  5,
       5,  8,  9,  9, 10, 10, 11, 13, 14, 16,
      17, 17, 18, 20, 21, 23, 25, 26, 28, 29,
      31, 33, 35, 37, 38, 40, 43, 45, 47, 49, },
    // Quartile
    {  1,  1,  2,  2,  4,  4,  6,  6,  8,  8,
       8, 10, 12, 16, 12, 17, 16, 18, 21, 20,
      23, 23, 25, 27, 29, 34, 34, 35, 38, 40,
      43, 45, 48, 51, 53, 56, 59, 62, 65, 68, },
    // High
    {  1,  1,  2,  4,  4,  4,  5,  6,  8,  8,
      11, 11, 16, 16, 18, 16, 19, 21, 25, 25,
      25, 34, 30, 32, 35, 37, 40, 42, 45, 48,
      51, 54, 57, 60, 63, 66, 70, 74, 77, 81, },
};

// Number of codewords inside the ECC block.
static const int32_t ErrorCorrectionCodewordsPerBlockCount[ErrorCorrectionLevelCount][VersionCount] = {
    // Low
    {  7, 10, 15, 20, 26, 18, 20, 24, 30, 18,
      20, 24, 26, 30, 22, 24, 28, 30, 28, 28,
      28, 28, 30, 30, 26, 28, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
    // Medium
    { 10, 16, 26, 18, 24, 16, 18, 22, 22, 26,
      30, 22, 22, 24, 24, 28, 28, 26, 26, 26,
      26, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28, },
    // Quartile
    { 13, 22, 18, 26, 18, 24, 18, 22, 20, 24,
      28, 26, 24, 20, 30, 24, 28, 28, 26, 30,
      28, 30, 30, 30, 30, 28, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
    // High
    { 17, 28, 22, 16, 22, 28, 26, 26, 24, 28,
      24, 28, 22, 24, 24, 30, 28, 28, 26, 28,
      30, 24, 30, 30, 30, 30, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
};

static const int32_t AlignmentCoordinates[VersionCount][AlignmentCoordinatesCount] = {
    { 0,  0,  0,  0,   0,   0,   0 },
    { 6, 18,  0,  0,   0,   0,   0 },
    { 6, 22,  0,  0,   0,   0,   0 },
    { 6, 26,  0,  0,   0,   0,   0 },
    { 6, 30,  0,  0,   0,   0,   0 },
    { 6, 34,  0,  0,   0,   0,   0 },
    { 6, 22, 38,  0,   0,   0,   0 },
    { 6, 24, 42,  0,   0,   0,   0 },
    { 6, 26, 46,  0,   0,   0,   0 },
    { 6, 28, 50,  0,   0,   0,   0 },
    { 6, 30, 54,  0,   0,   0,   0 },
    { 6, 32, 58,  0,   0,   0,   0 },
    { 6, 34, 62,  0,   0,   0,   0 },
    { 6, 26, 46, 66,   0,   0,   0 },
    { 6, 26, 48, 70,   0,   0,   0 },
    { 6, 26, 50, 74,   0,   0,   0 },
    { 6, 30, 54, 78,   0,   0,   0 },
    { 6, 30, 56, 82,   0,   0,   0 },
    { 6, 30, 58, 86,   0,   0,   0 },
    { 6, 34, 62, 90,   0,   0,   0 },
    { 6, 28, 50, 72,  94,   0,   0 },
    { 6, 26, 50, 74,  98,   0,   0 },
    { 6, 30, 54, 78, 102,   0,   0 },
    { 6, 28, 54, 80, 106,   0,   0 },
    { 6, 32, 58, 84, 110,   0,   0 },
    { 6, 30, 58, 86, 114,   0,   0 },
    { 6, 34, 62, 90, 118,   0,   0 },
    { 6, 26, 50, 74,  98, 122,   0 },
    { 6, 30, 54, 78, 102, 126,   0 },
    { 6, 26, 52, 78, 104, 130,   0 },
    { 6, 30, 56, 82, 108, 134,   0 },
    { 6, 34, 60, 86, 112, 138,   0 },
    { 6, 30, 58, 86, 114, 142,   0 },
    { 6, 34, 62, 90, 118, 146,   0 },
    { 6, 30, 54, 78, 102, 126, 150 },
    { 6, 24, 50, 76, 102, 128, 154 },
    { 6, 28, 54, 80, 106, 132, 158 },
    { 6, 32, 58, 84, 110, 136, 162 },
    { 6, 26, 54, 82, 110, 138, 166 },
    { 6, 30, 58, 86, 114, 142, 170 },
};

static const uint32_t FormatBits[ErrorCorrectionLevelCount][MaskCount] = {
    // Low
    { 0b111011111000100, 0b111001011110011, 0b111110110101010, 0b111100010011101,
      0b110011000101111, 0b110001100011000, 0b110110001000001, 0b110100101110110, },
    // Medium
    { 0b101010000010010, 0b101000100100101, 0b101111001111100, 0b101101101001011,
      0b100010111111001, 0b100000011001110, 0b100111110010111, 0b100101010100000, },
    // Quartile
    { 0b011010101011111, 0b011000001101000, 0b011111100110001, 0b011101000000110,
      0b010010010110100, 0b010000110000011, 0b010111011011010, 0b010101111101101, },
    // High
    { 0b001011010001001, 0b001001110111110, 0b001110011100111, 0b001100111010000,
      0b000011101100010, 0b000001001010101, 0b000110100001100, 0b000100000111011, },
};

int32_t qr_calc_data_codewords_count(int32_t version,
                                     ErrorCorrectionLevel level);
int32_t qr_find_first_unmatch_index(const char *text,
                                    int32_t textCharCount,
                                    char *charsToMatch,
                                    int32_t offset);
EncodingMode qr_get_encoding_mode(const char *text,
                                  int32_t textCharCount);
int32_t qr_get_version(EncodingMode mode,
                       ErrorCorrectionLevel level,
                       int32_t textCharCount);
void qr_draw_module(uint8_t *qrCode,
                    int32_t qrSize,
                    int32_t row,
                    int32_t column,
                    ModuleFlag value);
void qr_draw_rectangle(uint8_t *qrCode,
                       int32_t qrSize,
                       int32_t row,
                       int32_t column,
                       int32_t width,
                       int32_t height,
                       ModuleFlag value);
void qr_draw_square(uint8_t *qrCode,
                    int32_t qrSize,
                    int32_t row,
                    int32_t column,
                    int32_t size,
                    ModuleFlag value);
void qr_draw_finder_pattern(uint8_t *qrCode,
                            int32_t qrSize,
                            int32_t row,
                            int32_t column);
void qr_draw_finder_patterns(uint8_t *qrCode,
                             int32_t qrSize);
void qr_draw_alignment_pattern(uint8_t *qrCode,
                               int32_t qrSize,
                               int32_t row,
                               int32_t column);
void qr_draw_alignment_patterns(uint8_t *qrCode,
                                int32_t qrSize,
                                int32_t version);
void qr_draw_timing_patterns(uint8_t *qrCode,
                             int32_t qrSize);
void qr_draw_data(uint8_t *qrCode,
                  int32_t qrSize,
                  uint8_t *codewords,
                  int32_t codewordsCount);
void qr_print(FILE *out,
              uint8_t *qrCode,
              int32_t qrSize);
void qr_apply_mask(uint8_t *qrCode,
                   int32_t qrSize,
                   int32_t mask);
void qr_draw_format_bits(uint8_t *qrCode,
                         int32_t qrSize,
                         ErrorCorrectionLevel level,
                         int32_t mask);

#endif //QR_H
