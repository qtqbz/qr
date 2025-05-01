#define MIN_VERSION 0
#define MAX_VERSION 39
#define VERSION_COUNT 40
#define VERSION_INVALID (-1)

#define MAX_BLOCKS_COUNT 81
#define MAX_BLOCKS_LENGTH 153

#define MAX_MODULE_COUNT 31329 // = (4 × MaxVersion + 21) * (4 × MaxVersion + 21)

#define ALIGNMENT_COORDINATES_COUNT 7

#define MASK_COUNT 8

#define QR_MODULE_VALUE(qr, row, column) ((qr)->modules[(row) * (qr)->size + (column)])
#define QR_MODULE_COLOR(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).color)
#define QR_MODULE_TYPE(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).type)

typedef int32_t EncodingMode;
enum EncodingMode
{
    EM_NUMERIC,
    EM_ALPHANUM,
    EM_BYTE,
    //EM_KANJI,

    EM_COUNT,
};

static const char *const EncodingModeNames[EM_COUNT] = {
    "NUMERIC (0001)",
    "ALPHANUMERIC (0010)",
    "BYTE (0100)",
    //"KANJI (1000)",
};

typedef int32_t ErrorCorrectionLevel;
enum ErrorCorrectionLevel
{
    ECL_LOW,      // ~7%
    ECL_MEDIUM,   // ~15%
    ECL_QUARTILE, // ~25%
    ECL_HIGH,     // ~30%

    ECL_COUNT,
};

static const char *const ErrorCorrectionLevelNames[ECL_COUNT] = {
    "LOW",
    "MEDIUM",
    "QUARTILE",
    "HIGH",
};

typedef int32_t ModuleType;
enum ModuleType
{
    MT_NONE,
    MT_DATA,
    MT_FUNCTIONAL,
};

typedef int32_t ModuleColor;
enum ModuleColor
{
    MC_WHITE,
    MC_BLACK,
};

typedef struct ModuleValue ModuleValue;
struct ModuleValue
{
    ModuleType type;
    ModuleColor color;
};

typedef struct QR QR;
struct QR
{
    ModuleValue modules[MAX_MODULE_COUNT];
    int32_t size;
};

static const ModuleValue DATA_WHITE = { .type = MT_DATA, .color = MC_WHITE };
static const ModuleValue DATA_BLACK = { .type = MT_DATA, .color = MC_BLACK };
static const ModuleValue FUNCTIONAL_WHITE = { .type = MT_FUNCTIONAL, .color = MC_WHITE };
static const ModuleValue FUNCTIONAL_BLACK = { .type = MT_FUNCTIONAL, .color = MC_BLACK };

// Maximum length of the encoded text.
static const int32_t MAX_CHAR_COUNT[EM_COUNT][ECL_COUNT][VERSION_COUNT] = {
    // EM_NUMERIC
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
    // EM_ALPHANUM
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
    // EM_BYTE
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
static const int32_t LENGTH_BITS_COUNT[EM_COUNT][VERSION_COUNT] = {
    // EM_NUMERIC
    { 10, 10, 10, 10, 10, 10, 10, 10, 10, 12,
      12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
      12, 12, 12, 12, 12, 12, 14, 14, 14, 14,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, },
    // EM_ALPHANUM
    {  9,  9,  9,  9,  9,  9,  9,  9,  9, 11,
      11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
      11, 11, 11, 11, 11, 11, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13, },
    // EM_BYTE
    {  8,  8,  8,  8,  8,  8,  8,  8,  8, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
      16, 16, 16, 16, 16, 16, 16, 16, 16, 16, },
};

// Number of data modules and error correction modules.
static const int32_t CONTENT_MODULES_COUNT[VERSION_COUNT] = {
      208,   359,   567,   807,  1079,  1383,  1568,  1936,  2336,  2768,
     3232,  3728,  4256,  4651,  5243,  5867,  6523,  7211,  7931,  8683,
     9252, 10068, 10916, 11796, 12708, 13652, 14628, 15371, 16411, 17483,
    18587, 19723, 20891, 22091, 23008, 24272, 25568, 26896, 28256, 29648,
};

// Number of ECC blocks.
static const int32_t ERROR_CORRECTION_BLOCKS_COUNT[ECL_COUNT][VERSION_COUNT] = {
    // ECL_LOW
    {  1,  1,  1,  1,  1,  2,  2,  2,  2,  4,
       4,  4,  4,  4,  6,  6,  6,  6,  7,  8,
       8,  9,  9, 10, 12, 12, 12, 13, 14, 15,
      16, 17, 18, 19, 19, 20, 21, 22, 24, 25, },
    // ECL_MEDIUM
    {  1,  1,  1,  2,  2,  4,  4,  4,  5,  5,
       5,  8,  9,  9, 10, 10, 11, 13, 14, 16,
      17, 17, 18, 20, 21, 23, 25, 26, 28, 29,
      31, 33, 35, 37, 38, 40, 43, 45, 47, 49, },
    // ECL_QUARTILE
    {  1,  1,  2,  2,  4,  4,  6,  6,  8,  8,
       8, 10, 12, 16, 12, 17, 16, 18, 21, 20,
      23, 23, 25, 27, 29, 34, 34, 35, 38, 40,
      43, 45, 48, 51, 53, 56, 59, 62, 65, 68, },
    // ECL_HIGH
    {  1,  1,  2,  4,  4,  4,  5,  6,  8,  8,
      11, 11, 16, 16, 18, 16, 19, 21, 25, 25,
      25, 34, 30, 32, 35, 37, 40, 42, 45, 48,
      51, 54, 57, 60, 63, 66, 70, 74, 77, 81, },
};

// Number of codewords inside the ECC block.
static const int32_t ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[ECL_COUNT][VERSION_COUNT] = {
    // ECL_LOW
    {  7, 10, 15, 20, 26, 18, 20, 24, 30, 18,
      20, 24, 26, 30, 22, 24, 28, 30, 28, 28,
      28, 28, 30, 30, 26, 28, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
    // ECL_MEDIUM
    { 10, 16, 26, 18, 24, 16, 18, 22, 22, 26,
      30, 22, 22, 24, 24, 28, 28, 26, 26, 26,
      26, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28, },
    // ECL_QUARTILE
    { 13, 22, 18, 26, 18, 24, 18, 22, 20, 24,
      28, 26, 24, 20, 30, 24, 28, 28, 26, 30,
      28, 30, 30, 30, 30, 28, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
    // ECL_HIGH
    { 17, 28, 22, 16, 22, 28, 26, 26, 24, 28,
      24, 28, 22, 24, 24, 30, 28, 28, 26, 28,
      30, 24, 30, 30, 30, 30, 30, 30, 30, 30,
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, },
};

static const int32_t ALIGNMENT_COORDINATES[VERSION_COUNT][ALIGNMENT_COORDINATES_COUNT] = {
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

static const uint32_t FORMAT_BITS[ECL_COUNT][MASK_COUNT] = {
    // ECL_LOW
    { 0b111011111000100, 0b111001011110011, 0b111110110101010, 0b111100010011101,
      0b110011000101111, 0b110001100011000, 0b110110001000001, 0b110100101110110, },
    // ECL_MEDIUM
    { 0b101010000010010, 0b101000100100101, 0b101111001111100, 0b101101101001011,
      0b100010111111001, 0b100000011001110, 0b100111110010111, 0b100101010100000, },
    // ECL_QUARTILE
    { 0b011010101011111, 0b011000001101000, 0b011111100110001, 0b011101000000110,
      0b010010010110100, 0b010000110000011, 0b010111011011010, 0b010101111101101, },
    // ECL_HIGH
    { 0b001011010001001, 0b001001110111110, 0b001110011100111, 0b001100111010000,
      0b000011101100010, 0b000001001010101, 0b000110100001100, 0b000100000111011, },
};

static const uint32_t VERSION_BITS[VERSION_COUNT]= {
    0,
    0,
    0,
    0,
    0,
    0,
    0b000111110010010100,
    0b001000010110111100,
    0b001001101010011001,
    0b001010010011010011,
    0b001011101111110110,
    0b001100011101100010,
    0b001101100001000111,
    0b001110011000001101,
    0b001111100100101000,
    0b010000101101111000,
    0b010001010001011101,
    0b010010101000010111,
    0b010011010100110010,
    0b010100100110100110,
    0b010101011010000011,
    0b010110100011001001,
    0b010111011111101100,
    0b011000111011000100,
    0b011001000111100001,
    0b011010111110101011,
    0b011011000010001110,
    0b011100110000011010,
    0b011101001100111111,
    0b011110110101110101,
    0b011111001001010000,
    0b100000100111010101,
    0b100001011011110000,
    0b100010100010111010,
    0b100011011110011111,
    0b100100101100001011,
    0b100101010000101110,
    0b100110101001100100,
    0b100111010101000001,
    0b101000110001101001,
};

static int32_t
qr_calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level)
{
    int32_t contentCodewordsCount = CONTENT_MODULES_COUNT[version] / 8;
    int32_t errorCorrectionCodewordsCount = ERROR_CORRECTION_BLOCKS_COUNT[level][version] * ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[level][version];
    int32_t dataCodewordsCount = contentCodewordsCount - errorCorrectionCodewordsCount;
    return dataCodewordsCount;
}

static int32_t
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

static EncodingMode
qr_get_encoding_mode(const char *text, int32_t textCharCount)
{
    int32_t i = qr_find_first_unmatch_index(text, textCharCount, "0123456789", 0);
    if (i < 0) {
        return EM_NUMERIC;
    }
    i = qr_find_first_unmatch_index(text, textCharCount, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:", i + 1);
    if (i < 0) {
        return EM_ALPHANUM;
    }
    return EM_BYTE;
}

static int32_t
qr_get_version(EncodingMode mode, ErrorCorrectionLevel level, int32_t textCharCount)
{
    for (int32_t version = MIN_VERSION; version <= MAX_VERSION; version++) {
        if (textCharCount <= MAX_CHAR_COUNT[mode][level][version]) {
            return version;
        }
    }
    return VERSION_INVALID;
}

static void
qr_draw_module(QR *qr, int32_t row, int32_t column, ModuleValue value)
{
    if ((0 <= row && row < qr->size) && (0 <= column && column < qr->size)) {
        QR_MODULE_VALUE(qr, row, column) = value;
    }
}

static void
qr_draw_rectangle(QR *qr, int32_t row, int32_t column, int32_t width, int32_t height, ModuleValue value)
{
    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            qr_draw_module(qr, row + i, column + j, value);
        }
    }
}

static void
qr_draw_square(QR *qr, int32_t row, int32_t column, int32_t size, ModuleValue value)
{
    qr_draw_rectangle(qr, row, column, size, size, value);
}

static void
qr_draw_finder_pattern(QR *qr, int32_t row, int32_t column)
{
    qr_draw_square(qr, row - 1, column - 1, 9, FUNCTIONAL_WHITE); // separator
    qr_draw_square(qr, row + 0, column + 0, 7, FUNCTIONAL_BLACK);
    qr_draw_square(qr, row + 1, column + 1, 5, FUNCTIONAL_WHITE);
    qr_draw_square(qr, row + 2, column + 2, 3, FUNCTIONAL_BLACK);
}

static void
qr_draw_finder_patterns(QR *qr)
{
    qr_draw_finder_pattern(qr, 0, 0);
    qr_draw_finder_pattern(qr, 0, qr->size - 7);
    qr_draw_finder_pattern(qr, qr->size - 7, 0);
}

static void
qr_draw_alignment_pattern(QR *qr, int32_t row, int32_t column)
{
    qr_draw_square(qr, row + 0, column + 0, 5, FUNCTIONAL_BLACK);
    qr_draw_square(qr, row + 1, column + 1, 3, FUNCTIONAL_WHITE);
    qr_draw_module(qr, row + 2, column + 2, FUNCTIONAL_BLACK);
}

static void
qr_draw_alignment_patterns(QR *qr, int32_t version)
{
    for (int32_t i = 0; i < ALIGNMENT_COORDINATES_COUNT; i++) {
        int32_t rowCenter = ALIGNMENT_COORDINATES[version][i];
        if (rowCenter == 0) {
            break;
        }
        for (int32_t j = 0; j < ALIGNMENT_COORDINATES_COUNT; j++) {
            int32_t columnCenter = ALIGNMENT_COORDINATES[version][j];
            if (columnCenter == 0) {
                break;
            }
            if (QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter - 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter + 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter - 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter + 2) == MT_NONE) {
                int32_t row = rowCenter - 2;
                int32_t column = columnCenter - 2;
                qr_draw_alignment_pattern(qr, row, column);
            }
        }
    }
}

static void
qr_draw_timing_patterns(QR *qr)
{
    ModuleValue value = FUNCTIONAL_BLACK;
    for (int32_t i = 8; i < (qr->size - 7); i++) {
        qr_draw_module(qr, 6, i, value);
        qr_draw_module(qr, i, 6, value);
        value.color = (value.color == MC_WHITE) ? MC_BLACK : MC_WHITE;
    }
}

static void
qr_draw_functional_patterns(QR *qr, int32_t version)
{
    qr_draw_finder_patterns(qr);
    qr_draw_alignment_patterns(qr, version);
    qr_draw_timing_patterns(qr);
    qr_draw_module(qr, 4 * version + 13, 8, FUNCTIONAL_BLACK); // dark module
}

static void
qr_reserve_format_modules(QR *qr)
{
    // Vertical format modules
    qr_draw_rectangle(qr, 0, 8, 1, 6, FUNCTIONAL_BLACK);
    qr_draw_rectangle(qr, 7, 8, 1, 2, FUNCTIONAL_BLACK);
    qr_draw_rectangle(qr, qr->size - 7, 8, 1, 8, FUNCTIONAL_BLACK);

    // Horizontal format modules
    qr_draw_rectangle(qr, 8, 0, 6, 1, FUNCTIONAL_BLACK);
    qr_draw_rectangle(qr, 8, 7, 2, 1, FUNCTIONAL_BLACK);
    qr_draw_rectangle(qr, 8, qr->size - 8, 8, 1, FUNCTIONAL_BLACK);
}

static void
qr_reserve_version_modules(QR *qr, int32_t version)
{
    if (version < 6) {
        return;
    }
    // Vertical version modules
    qr_draw_rectangle(qr, qr->size - 11, 0, 6, 3, FUNCTIONAL_BLACK);

    // Horizontal version modules
    qr_draw_rectangle(qr, 0, qr->size - 11, 3, 6, FUNCTIONAL_BLACK);
}

static void
qr_draw_data(QR *qr, uint8_t *codewords, int32_t codewordsCount)
{
    int32_t column = qr->size - 1;
    int32_t row = qr->size - 1;
    int32_t rowDirection = -1; // -1 == up, +1 == down
    int32_t codewordsIndex = 0;
    int32_t bitIndex = 7;

    while (row >= 0 && column >= 0) {
        if (QR_MODULE_TYPE(qr, row, column) == MT_NONE) {
            if (codewordsIndex < codewordsCount) {
                uint8_t codeword = codewords[codewordsIndex];
                QR_MODULE_VALUE(qr, row, column) = ((codeword >> bitIndex) & 1) ? DATA_BLACK : DATA_WHITE;
                if (bitIndex == 0) {
                    codewordsIndex++;
                    bitIndex = 7;
                }
                else {
                    bitIndex--;
                }
            }
            else {
                QR_MODULE_VALUE(qr, row, column) = DATA_WHITE;
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

static void
qr_print(FILE *out, QR *qr)
{
    for (int32_t i = -4; i <= qr->size + 4; i++) {
        for (int32_t j = -4; j <= qr->size + 4; j++) {
            if (i < 0 || i >= qr->size || j < 0 || j >= qr->size) {
                // Drawing frame
                fprintf(out, "\033[47m  \033[0m");
                continue;
            }

            if (QR_MODULE_COLOR(qr, i, j) == MC_WHITE) {
                fprintf(out, "\033[47m  \033[0m");
            }
            else {
                fprintf(out, "\033[40m  \033[0m");
            }
        }
        fprintf(out, "\n");
    }
}

static void
qr_apply_mask(QR *qr, int32_t mask)
{
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size; column++) {
            ModuleValue value = QR_MODULE_VALUE(qr, row, column);
            if (value.type != MT_DATA) {
                continue;
            }

            bool invert = false;
            switch (mask) {
                case 0: {
                    invert = ((row + column) % 2) == 0;
                } break;
                case 1: {
                    invert = (row % 2) == 0;
                } break;
                case 2: {
                    invert = (column % 3) == 0;
                } break;
                case 3: {
                    invert = ((row + column) % 3) == 0;
                } break;
                case 4: {
                    invert = ((row / 2 + column / 3) % 2) == 0;
                } break;
                case 5: {
                    invert = ((row * column) % 2) + ((row * column) % 3) == 0;
                } break;
                case 6: {
                    invert = (((row * column) % 2) + ((row * column) % 3) % 2) == 0;
                } break;
                case 7: {
                    invert = (((row + column) % 2) + ((row * column) % 3) % 2) == 0;
                } break;
                default: {
                    ASSERT(false); // unreachable
                }
            }
            if (invert) {
                QR_MODULE_COLOR(qr, row, column) = (value.color == MC_WHITE) ? MC_BLACK : MC_WHITE;
            }
        }
    }
}

static int32_t
qr_calc_rule1_penalty(QR *qr)
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
qr_calc_rule2_penalty(QR *qr)
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
qr_calc_rule3_penalty(QR *qr)
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
                int32_t preWhiteModuleCount = 0;
                int32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((column - i) < 0) || (QR_MODULE_COLOR(qr, row, column - i) == MC_WHITE)) {
                        preWhiteModuleCount++;
                    }
                    if (((column + 6 + i) >= qr->size) || (QR_MODULE_COLOR(qr, row, column + 6 + i) == MC_WHITE)) {
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
                int32_t preWhiteModuleCount = 0;
                int32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((row - i) < 0) || (QR_MODULE_COLOR(qr, row - i, column) == MC_WHITE)) {
                        preWhiteModuleCount++;
                    }
                    if (((row + 6 + i) >= qr->size) || (QR_MODULE_COLOR(qr, row + 6 + i, column) == MC_WHITE)) {
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
qr_calc_rule4_penalty(QR *qr)
{
    int32_t totalModuleCount = qr->size * qr->size;
    int32_t blackModuleCount = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size - 11; column++) {
            if (QR_MODULE_COLOR(qr, row, column) == MC_BLACK) {
                blackModuleCount++;
            }
        }
    }
    int32_t fivePercentVariances = abs(blackModuleCount * 2 - totalModuleCount) * 10 / totalModuleCount;
    int32_t penalty = fivePercentVariances * 10;
    return penalty;
}

static int32_t
qr_calc_mask_penalty(QR *qr)
{
    int32_t penalty1 = qr_calc_rule1_penalty(qr);
    int32_t penalty2 = qr_calc_rule2_penalty(qr);
    int32_t penalty3 = qr_calc_rule3_penalty(qr);
    int32_t penalty4 = qr_calc_rule4_penalty(qr);
    return penalty1 + penalty2 + penalty3 + penalty4;
}

static void
qr_draw_format_modules(QR *qr, ErrorCorrectionLevel level, int32_t mask)
{
    uint32_t formatBits = FORMAT_BITS[level][mask];
    int32_t bitIndex = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        if (row == 6) {
            row++; // skip timing pattern module
        }
        else if (row == 9) {
            row = qr->size - 7; // skip to the opposite side
        }
        qr_draw_module(qr, row, 8, ((formatBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE);
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
        qr_draw_module(qr, 8, column, ((formatBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE);
        bitIndex--;
    }
}

static void
qr_draw_version_modules(QR *qr, int32_t version)
{
    ASSERT(MIN_VERSION <= version && version <= MAX_VERSION);

    if (version < 6) {
        return;
    }
    uint32_t versionBits = VERSION_BITS[version];
    int32_t bitIndex = 0;

    for (int32_t i = 0; i < 6; i++) {
        for (int32_t j = 0; j < 3; j++) {
            ModuleValue value = ((versionBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE;
            qr_draw_module(qr, i, qr->size - 11 + j, value);
            qr_draw_module(qr, qr->size - 11 + j, i, value);
            bitIndex++;
        }
    }
}
