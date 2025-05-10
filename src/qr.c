#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "bv.h"
#include "gf256.h"
#include "qr.h"

#define MAX_BLOCKS_COUNT 81
#define MAX_BLOCKS_LENGTH 153

#define ALIGNMENT_COORDINATES_COUNT 7

#define MAX_POLYNOM_DEGREE 123 // = max data codewords per block
#define MAX_GENERATOR_POLYNOM_DEGREE 30 // = max error correction codewords per block

#define QR_MODULE_VALUE(qr, row, column) ((qr)->matrix[(row) * (qr)->size + (column)])
#define QR_MODULE_COLOR(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).color)
#define QR_MODULE_TYPE(qr, row, column) ((QR_MODULE_VALUE((qr), (row), (column))).type)

global const ModuleValue DATA_WHITE = { .type = MT_DATA, .color = MC_WHITE };
global const ModuleValue DATA_BLACK = { .type = MT_DATA, .color = MC_BLACK };
global const ModuleValue FUNCTIONAL_WHITE = { .type = MT_FUNCTIONAL, .color = MC_WHITE };
global const ModuleValue FUNCTIONAL_BLACK = { .type = MT_FUNCTIONAL, .color = MC_BLACK };

// Maximum length of the encoded text.
global const int32_t MAX_CHAR_COUNT[EM_COUNT][ECL_COUNT][VERSION_COUNT] = {
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
global const int32_t LENGTH_BITS_COUNT[EM_COUNT][VERSION_COUNT] = {
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
global const int32_t CONTENT_MODULES_COUNT[VERSION_COUNT] = {
      208,   359,   567,   807,  1079,  1383,  1568,  1936,  2336,  2768,
     3232,  3728,  4256,  4651,  5243,  5867,  6523,  7211,  7931,  8683,
     9252, 10068, 10916, 11796, 12708, 13652, 14628, 15371, 16411, 17483,
    18587, 19723, 20891, 22091, 23008, 24272, 25568, 26896, 28256, 29648,
};

// Number of ECC blocks.
global const int32_t ERROR_CORRECTION_BLOCKS_COUNT[ECL_COUNT][VERSION_COUNT] = {
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
global const int32_t ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[ECL_COUNT][VERSION_COUNT] = {
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

global const int32_t ALIGNMENT_COORDINATES[VERSION_COUNT][ALIGNMENT_COORDINATES_COUNT] = {
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

global const uint32_t FORMAT_BITS[ECL_COUNT][MASK_COUNT] = {
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

global const uint32_t VERSION_BITS[VERSION_COUNT]= {
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

global const uint8_t GENERATOR_POLYNOM[MAX_GENERATOR_POLYNOM_DEGREE + 1][MAX_GENERATOR_POLYNOM_DEGREE + 1] = {
    {  1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,   3,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,   7,  14,   8,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  15,  54, 120,  64,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  31, 198,  63, 147, 116,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  63,   1, 218,  32, 227,  38,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 127, 122, 154, 164,  11,  68, 117,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 255,  11,  81,  54, 239, 173, 200,  24,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 226, 207, 158, 245, 235, 164, 232, 197,  37,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 216, 194, 159, 111, 199,  94,  95, 113, 157, 193,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 172, 130, 163,  50, 123, 219, 162, 248, 144, 116, 160,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  68, 119,  67, 118, 220,  31,   7,  84,  92, 127, 213,  97,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 137,  73, 227,  17, 177,  17,  52,  13,  46,  43,  83, 132, 120,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  14,  54, 114,  70, 174, 151,  43, 158, 195, 127, 166, 210, 234, 163,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  29, 196, 111, 163, 112,  74,  10, 105, 105, 139, 132, 151,  32, 134,  26,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  59,  13, 104, 189,  68, 209,  30,   8, 163,  65,  41, 229,  98,  50,  36,  59,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 119,  66,  83, 120, 119,  22, 197,  83, 249,  41, 143, 134,  85,  53, 125,  99,  79,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 239, 251, 183, 113, 149, 175, 199, 215, 240, 220,  73,  82, 173,  75,  32,  67, 217, 146,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 194,   8,  26, 146,  20, 223, 187, 152,  85, 115, 238, 133, 146, 109, 173, 138,  33, 172, 179,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 152, 185, 240,   5, 111,  99,   6, 220, 112, 150,  69,  36, 187,  22, 228, 198, 121, 121, 165, 174,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  44, 243,  13, 131,  49, 132, 194,  67, 214,  28,  89, 124,  82, 158, 244,  37, 236, 142,  82, 255,  89,   0,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1,  89, 179, 131, 176, 182, 244,  19, 189,  69,  40,  28, 137,  29, 123,  67, 253,  86, 218, 230,  26, 145, 245,   0,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 179,  68, 154, 163, 140, 136, 190, 152,  25,  85,  19,   3, 196,  27, 113, 198,  18, 130,   2, 120,  93,  41,  71,   0,   0,   0,   0,   0,   0,   0, },
    {  1, 122, 118, 169,  70, 178, 237, 216, 102, 115, 150, 229,  73, 130,  72,  61,  43, 206,   1, 237, 247, 127, 217, 144, 117,   0,   0,   0,   0,   0,   0, },
    {  1, 245,  49, 228,  53, 215,   6, 205, 210,  38,  82,  56,  80,  97, 139,  81, 134, 126, 168,  98, 226, 125,  23, 171, 173, 193,   0,   0,   0,   0,   0, },
    {  1, 246,  51, 183,   4, 136,  98, 199, 152,  77,  56, 206,  24, 145,  40, 209, 117, 233,  42, 135,  68,  70, 144, 146,  77,  43,  94,   0,   0,   0,   0, },
    {  1, 240,  61,  29, 145, 144, 117, 150,  48,  58, 139,  94, 134, 193, 105,  33, 169, 202, 102, 123, 113, 195,  25, 213,   6, 152, 164, 217,   0,   0,   0, },
    {  1, 252,   9,  28,  13,  18, 251, 208, 150, 103, 174, 100,  41, 167,  12, 247,  56, 117, 119, 233, 127, 181, 100, 121, 147, 176,  74,  58, 197,   0,   0, },
    {  1, 228, 193, 196,  48, 170,  86,  80, 217,  54, 143,  79,  32,  88, 255,  87,  24,  15, 251,  85,  82, 201,  58, 112, 191, 153, 108, 132, 143, 170,   0, },
    {  1, 212, 246,  77,  73, 195, 192,  75,  98,   5,  70, 103, 177,  22, 217, 138,  51, 181, 246,  72,  25,  18,  46, 228,  74, 216, 195,  11, 106, 130, 150, },
};

internal int32_t
calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level)
{
    int32_t contentCodewordsCount = CONTENT_MODULES_COUNT[version] / 8;
    int32_t errorCorrectionCodewordsCount = ERROR_CORRECTION_BLOCKS_COUNT[level][version]
        * ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[level][version];
    int32_t dataCodewordsCount = contentCodewordsCount - errorCorrectionCodewordsCount;
    return dataCodewordsCount;
}

internal int32_t
find_first_unmatch_index(char *text, int32_t textLen, char *charsToMatch, int32_t offset)
{
    for (int32_t i = offset; i < textLen; i++) {
        char ch = text[i];
        if (!strchr(charsToMatch, ch)) {
            return i;
        }
    }
    return -1;
}

internal EncodingMode
get_encoding_mode(char *text, int32_t textLen)
{
    int32_t i = find_first_unmatch_index(text, textLen, "0123456789", 0);
    if (i < 0) {
        return EM_NUMERIC;
    }
    i = find_first_unmatch_index(text, textLen, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:", i + 1);
    if (i < 0) {
        return EM_ALPHANUM;
    }
    return EM_BYTE;
}

internal int32_t
get_version(EncodingMode mode, ErrorCorrectionLevel level, int32_t textCharCount)
{
    for (int32_t version = MIN_VERSION; version <= MAX_VERSION; version++) {
        if (textCharCount <= MAX_CHAR_COUNT[mode][level][version]) {
            return version;
        }
    }
    return VERSION_INVALID;
}

internal void
draw_module(QR *qr, int32_t row, int32_t column, ModuleValue value)
{
    if ((0 <= row && row < qr->size) && (0 <= column && column < qr->size)) {
        QR_MODULE_VALUE(qr, row, column) = value;
    }
}

internal void
draw_rectangle(QR *qr, int32_t row, int32_t column, int32_t width, int32_t height, ModuleValue value)
{
    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            draw_module(qr, row + i, column + j, value);
        }
    }
}

internal void
draw_square(QR *qr, int32_t row, int32_t column, int32_t size, ModuleValue value)
{
    draw_rectangle(qr, row, column, size, size, value);
}

internal void
draw_finder_pattern(QR *qr, int32_t row, int32_t column)
{
    draw_square(qr, row - 1, column - 1, 9, FUNCTIONAL_WHITE); // separator
    draw_square(qr, row + 0, column + 0, 7, FUNCTIONAL_BLACK);
    draw_square(qr, row + 1, column + 1, 5, FUNCTIONAL_WHITE);
    draw_square(qr, row + 2, column + 2, 3, FUNCTIONAL_BLACK);
}

internal void
draw_finder_patterns(QR *qr)
{
    draw_finder_pattern(qr, 0, 0);
    draw_finder_pattern(qr, 0, qr->size - 7);
    draw_finder_pattern(qr, qr->size - 7, 0);
}

internal void
draw_alignment_pattern(QR *qr, int32_t row, int32_t column)
{
    draw_square(qr, row + 0, column + 0, 5, FUNCTIONAL_BLACK);
    draw_square(qr, row + 1, column + 1, 3, FUNCTIONAL_WHITE);
    draw_module(qr, row + 2, column + 2, FUNCTIONAL_BLACK);
}

internal void
draw_alignment_patterns(QR *qr)
{
    for (int32_t i = 0; i < ALIGNMENT_COORDINATES_COUNT; i++) {
        int32_t rowCenter = ALIGNMENT_COORDINATES[qr->version][i];
        if (rowCenter == 0) {
            break;
        }
        for (int32_t j = 0; j < ALIGNMENT_COORDINATES_COUNT; j++) {
            int32_t columnCenter = ALIGNMENT_COORDINATES[qr->version][j];
            if (columnCenter == 0) {
                break;
            }
            if (QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter - 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter - 2, columnCenter + 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter - 2) == MT_NONE
                && QR_MODULE_TYPE(qr, rowCenter + 2, columnCenter + 2) == MT_NONE) {
                int32_t row = rowCenter - 2;
                int32_t column = columnCenter - 2;
                draw_alignment_pattern(qr, row, column);
            }
        }
    }
}

internal void
draw_timing_patterns(QR *qr)
{
    ModuleValue value = FUNCTIONAL_BLACK;
    for (int32_t i = 8; i < (qr->size - 7); i++) {
        draw_module(qr, 6, i, value);
        draw_module(qr, i, 6, value);
        value.color = (value.color == MC_WHITE) ? MC_BLACK : MC_WHITE;
    }
}

internal void
draw_functional_patterns(QR *qr)
{
    draw_finder_patterns(qr);
    draw_alignment_patterns(qr);
    draw_timing_patterns(qr);
    draw_module(qr, 4 * qr->version + 13, 8, FUNCTIONAL_BLACK); // dark module
}

internal void
reserve_format_modules(QR *qr)
{
    // Vertical format modules
    draw_rectangle(qr, 0, 8, 1, 6, FUNCTIONAL_BLACK);
    draw_rectangle(qr, 7, 8, 1, 2, FUNCTIONAL_BLACK);
    draw_rectangle(qr, qr->size - 7, 8, 1, 8, FUNCTIONAL_BLACK);

    // Horizontal format modules
    draw_rectangle(qr, 8, 0, 6, 1, FUNCTIONAL_BLACK);
    draw_rectangle(qr, 8, 7, 2, 1, FUNCTIONAL_BLACK);
    draw_rectangle(qr, 8, qr->size - 8, 8, 1, FUNCTIONAL_BLACK);
}

internal void
reserve_version_modules(QR *qr)
{
    if (qr->version < 6) {
        return;
    }
    // Vertical version modules
    draw_rectangle(qr, qr->size - 11, 0, 6, 3, FUNCTIONAL_BLACK);

    // Horizontal version modules
    draw_rectangle(qr, 0, qr->size - 11, 3, 6, FUNCTIONAL_BLACK);
}

internal void
draw_data(QR *qr, uint8_t *codewords, int32_t codewordsCount)
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

internal void
apply_mask(QR *qr, int32_t mask)
{
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size; column++) {
            ModuleValue value = QR_MODULE_VALUE(qr, row, column);
            if (value.type != MT_DATA) {
                continue;
            }

            bool invert = false;
            switch (mask) {
                case 0: invert = ((row + column) % 2) == 0; break;
                case 1: invert = (row % 2) == 0; break;
                case 2: invert = (column % 3) == 0; break;
                case 3: invert = ((row + column) % 3) == 0; break;
                case 4: invert = (((row / 2) + (column / 3)) % 2) == 0; break;
                case 5: invert = ((row * column) % 2) + ((row * column) % 3) == 0; break;
                case 6: invert = ((((row * column) % 2) + ((row * column) % 3)) % 2) == 0; break;
                case 7: invert = ((((row + column) % 2) + ((row * column) % 3)) % 2) == 0; break;
                default: UNREACHABLE();
            }
            if (invert) {
                QR_MODULE_COLOR(qr, row, column) = (value.color == MC_WHITE) ? MC_BLACK : MC_WHITE;
            }
        }
    }
}

internal int32_t
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

internal int32_t
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

internal int32_t
calc_rule3_penalty(QR *qr)
{
    int32_t penalty = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size - 6; column++) {
            if ((QR_MODULE_COLOR(qr, row, column + 0) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row, column + 1) == MC_WHITE)
                && (QR_MODULE_COLOR(qr, row, column + 2) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row, column + 3) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row, column + 4) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row, column + 5) == MC_WHITE)
                && (QR_MODULE_COLOR(qr, row, column + 6) == MC_BLACK)) {
                int32_t preWhiteModuleCount = 0;
                int32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((column - i) >= 0) && (QR_MODULE_COLOR(qr, row, column - i) == MC_WHITE)) {
                        preWhiteModuleCount++;
                    }
                    if (((column + 6 + i) < qr->size) && (QR_MODULE_COLOR(qr, row, column + 6 + i) == MC_WHITE)) {
                        postWhiteModuleCount++;
                    }
                }
                if (preWhiteModuleCount == 4) {
                    penalty += 40;
                }
                if (postWhiteModuleCount == 4) {
                    penalty += 40;
                }
            }
        }
    }
    for (int32_t column = 0; column < qr->size; column++) {
        for (int32_t row = 0; row < qr->size - 6; row++) {
            if ((QR_MODULE_COLOR(qr, row + 0, column) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row + 1, column) == MC_WHITE)
                && (QR_MODULE_COLOR(qr, row + 2, column) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row + 3, column) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row + 4, column) == MC_BLACK)
                && (QR_MODULE_COLOR(qr, row + 5, column) == MC_WHITE)
                && (QR_MODULE_COLOR(qr, row + 6, column) == MC_BLACK)) {
                int32_t preWhiteModuleCount = 0;
                int32_t postWhiteModuleCount = 0;
                for (int32_t i = 1; i <= 4; i++) {
                    if (((row - i) >= 0) && (QR_MODULE_COLOR(qr, row - i, column) == MC_WHITE)) {
                        preWhiteModuleCount++;
                    }
                    if (((row + 6 + i) < qr->size) && (QR_MODULE_COLOR(qr, row + 6 + i, column) == MC_WHITE)) {
                        postWhiteModuleCount++;
                    }
                }
                if (preWhiteModuleCount == 4) {
                    penalty += 40;
                }
                if (postWhiteModuleCount == 4) {
                    penalty += 40;
                }
            }
        }
    }
    return penalty;
}

internal int32_t
calc_rule4_penalty(QR *qr)
{
    int32_t totalModuleCount = qr->size * qr->size;
    int32_t blackModuleCount = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        for (int32_t column = 0; column < qr->size; column++) {
            if (QR_MODULE_COLOR(qr, row, column) == MC_BLACK) {
                blackModuleCount++;
            }
        }
    }
    int32_t fivePercentVariances = abs(blackModuleCount * 2 - totalModuleCount) * 10 / totalModuleCount;
    int32_t penalty = fivePercentVariances * 10;
    return penalty;
}

internal int32_t
calc_mask_penalty(QR *qr)
{
    int32_t penalty1 = calc_rule1_penalty(qr);
    int32_t penalty2 = calc_rule2_penalty(qr);
    int32_t penalty3 = calc_rule3_penalty(qr);
    int32_t penalty4 = calc_rule4_penalty(qr);
    return penalty1 + penalty2 + penalty3 + penalty4;
}

internal void
draw_format_modules(QR *qr, int32_t mask)
{
    uint32_t formatBits = FORMAT_BITS[qr->level][mask];
    int32_t bitIndex = 0;
    for (int32_t row = 0; row < qr->size; row++) {
        if (row == 6) {
            row++; // skip timing pattern module
        }
        else if (row == 9) {
            row = qr->size - 7; // skip to the opposite side
        }
        draw_module(qr, row, 8, ((formatBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE);
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
        draw_module(qr, 8, column, ((formatBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE);
        bitIndex--;
    }
}

internal void
draw_version_modules(QR *qr)
{
    ASSERT(MIN_VERSION <= qr->version && qr->version <= MAX_VERSION);

    if (qr->version < 6) {
        return;
    }
    uint32_t versionBits = VERSION_BITS[qr->version];
    int32_t bitIndex = 0;

    for (int32_t i = 0; i < 6; i++) {
        for (int32_t j = 0; j < 3; j++) {
            ModuleValue value = ((versionBits >> bitIndex) & 1) ? FUNCTIONAL_BLACK : FUNCTIONAL_WHITE;
            draw_module(qr, i, qr->size - 11 + j, value);
            draw_module(qr, qr->size - 11 + j, i, value);
            bitIndex++;
        }
    }
}

internal QR
analyse_data(char *text, int32_t textLen, ErrorCorrectionLevel forcedLevel, int32_t forcedVersion, bool isDebug)
{
    QR qr = {};

    // 1. Data analysis.
    qr.mode = get_encoding_mode(text, textLen);

    qr.level = (forcedLevel != LEVEL_INVALID) ? forcedLevel : ECL_LOW;

    qr.version = (forcedVersion != VERSION_INVALID) ? forcedVersion : get_version(qr.mode, qr.level, textLen);
    if (qr.version == VERSION_INVALID) {
        fprintf(stderr,
                "Failed to find a suitable version for a text of length %d "
                "with %s mode "
                "and %s error correction level\n",
                textLen,
                EncodingModeNames[qr.mode],
                ErrorCorrectionLevelNames[qr.level]);
        exit(1);
    }

    if (forcedLevel == LEVEL_INVALID) {
        // Try to increase the error correction level while still staying in the same version.
        for (ErrorCorrectionLevel newLevel = qr.level + 1; newLevel <= ECL_HIGH; newLevel++) {
            if (textLen > MAX_CHAR_COUNT[qr.mode][newLevel][qr.version]) {
                break;
            }
            qr.level = newLevel;
        }
    }

    if (textLen > MAX_CHAR_COUNT[qr.mode][qr.level][qr.version]) {
        fprintf(stderr,
                "The text exceeds the length limit of %d characters "
                "for %s mode "
                "and %s error correction level "
                "and version %d\n",
                MAX_CHAR_COUNT[qr.mode][qr.level][qr.version],
                EncodingModeNames[qr.mode],
                ErrorCorrectionLevelNames[qr.level],
                qr.version);
        exit(1);
    }

    if (isDebug) {
        fprintf(stderr, ">>> DATA ANALYSIS\n");
        fprintf(stderr, "Text: ");
        for (int32_t i = 0; i < textLen; i++) {
            switch (text[i]) {
                case '\a': fprintf(stderr, "\\a"); break;
                case '\b': fprintf(stderr, "\\b"); break;
                case '\f': fprintf(stderr, "\\f"); break;
                case '\n': fprintf(stderr, "\\n"); break;
                case '\r': fprintf(stderr, "\\r"); break;
                case '\t': fprintf(stderr, "\\t"); break;
                case '\v': fprintf(stderr, "\\v"); break;
                case ' ': fprintf(stderr, "\\ "); break;
                default: fprintf(stderr, "%c", text[i]);
            }
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "Text length: %d\n", textLen);
        fprintf(stderr, "Encoding mode: %s\n", EncodingModeNames[qr.mode]);
        fprintf(stderr, "QR version: %d\n", qr.version + 1);
        fprintf(stderr, "Error correction level: %s\n", ErrorCorrectionLevelNames[qr.level]);
        fprintf(stderr, "\n");
    }
    return qr;
}

internal BitVec
encode_data(QR* qr, char *text, int32_t textLen, bool isDebug)
{
    // 2. Data Encoding
    BitVec bv = {};

    // Encoding mode
    bv_append(&bv, (1U) << qr->mode, 4);

    // Text length
    int32_t lengthBitCount = LENGTH_BITS_COUNT[qr->mode][qr->version];
    bv_append(&bv, textLen, lengthBitCount);

    // Text itself
    if (qr->mode == EM_NUMERIC) {
        int32_t number = 0;
        for (int32_t i = 0; i < textLen; i++) {
            uint8_t ch = text[i];
            int32_t digit = ch - '0';
            number = number * 10 + digit;
            if (((i % 3) == 2) || (i == (textLen - 1))) {
                bv_append(&bv, number, (number >= 100) ? 10 : ((number >= 10) ? 7 : 4));
                number = 0;
            }
        }
    }
    else if (qr->mode == EM_ALPHANUM) {
        int32_t number = 0;
        for (int32_t i = 0; i < textLen; i++) {
            uint8_t ch = text[i];

            int32_t addend = 0;
            if (ch >= '0' && ch <= '9') addend = ch - '0';
            else if (ch >= 'A' && ch <= 'Z') addend = ch - 'A' + 10;
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
            if (((i % 2) == 1) || (i == (textLen - 1))) {
                bv_append(&bv, number, ((i % 2) == 1) ? 11 : 6);
                number = 0;
            }
        }
    }
    else {
        for (int32_t i = 0; i < textLen; i++) {
            bv_append(&bv, text[i], 8);
        }
    }

    int32_t dataCodewordsCount = calc_data_codewords_count(qr->version, qr->level);
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

    if (isDebug) {
        fprintf(stderr, ">>> DATA ENCODING\n");

        fprintf(stderr, "Data codewords in binary: ");
        bv_print_bin(stderr, bv);

        fprintf(stderr, "Data codewords in hex: ");
        bv_print_hex(stderr, bv);

        fprintf(stderr, "\n");
    }

    return bv;
}

internal int32_t
prepare_codewords(QR *qr, BitVec *bv, uint8_t *codewords, bool isDebug)
{
    // 3. Error correction coding
    uint8_t blocks[MAX_BLOCKS_COUNT][MAX_BLOCKS_LENGTH] = {};

    // Divide data codewords into blocks and calculate error correction codewords for them
    uint8_t *dataCodewords = bv->bytes;
    int32_t contentCodewordsCount = CONTENT_MODULES_COUNT[qr->version] / 8;

    int32_t blocksCount = ERROR_CORRECTION_BLOCKS_COUNT[qr->level][qr->version];
    ASSERT(blocksCount <= MAX_BLOCKS_COUNT);

    int32_t shortBlocksCount = blocksCount - (contentCodewordsCount % blocksCount);
    int32_t shortBlockLength = contentCodewordsCount / blocksCount;
    ASSERT(shortBlockLength <= MAX_BLOCKS_LENGTH);

    int32_t errorCodewordsPerBlockCount = ERROR_CORRECTION_CODEWORDS_PER_BLOCK_COUNT[qr->level][qr->version];
    uint8_t *divisor = (uint8_t *)GENERATOR_POLYNOM[errorCodewordsPerBlockCount];

    int32_t dataCodewordsIndex = 0;
    uint8_t errorCodewords[MAX_POLYNOM_DEGREE + MAX_GENERATOR_POLYNOM_DEGREE + 1] = {};
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
        int32_t remainderLength = gf256_polynom_divide(block,
                                                       currBlockLength,
                                                       divisor,
                                                       errorCodewordsPerBlockCount + 1,
                                                       errorCodewords);
        ASSERT(remainderLength == errorCodewordsPerBlockCount);

        // Copy error correcting codewords to block
        int32_t errorCodewordsIndex = 0;
        for (int32_t j = 0; j < errorCodewordsPerBlockCount; j++) {
            block[blockIndex++] = errorCodewords[errorCodewordsIndex++];
        }
    }

    // Interleave codewords
    int32_t codewordsCount = 0;
    for (int32_t i = 0; i < (shortBlockLength + 1); i++) {
        for (int32_t j = 0; j < blocksCount; j++) {
            if (j < shortBlocksCount && i == (shortBlockLength - errorCodewordsPerBlockCount)) {
                // skip the padding on shorter blocks
                continue;
            }
            codewords[codewordsCount++] = blocks[j][i];
        }
    }

    if (isDebug) {
        fprintf(stderr, ">>> ERROR CORRECTION CODE GENERATION\n");

        fprintf(stderr, "Block count: %d\n", blocksCount);
        for (int32_t i = 0; i < blocksCount; i++) {
            fprintf(stderr, "Block #%d: ", i);
            uint8_t *block = blocks[i];
            int32_t currBlockLength = (i < shortBlocksCount) ? shortBlockLength : shortBlockLength + 1;
            for (int32_t j = 0; j < currBlockLength; j++) {
                fprintf(stderr, "%02X ", block[j]);
            }
            fprintf(stderr, "\n");
        }

        fprintf(stderr, "Interleaved codewords: ");
        for (int32_t i = 0; i < codewordsCount; i++) {
            fprintf(stderr, "%02X ", codewords[i]);
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "\n");
    }

    return codewordsCount;
}

internal void
draw_matrix(QR *qr,
            uint8_t *codewords,
            int32_t codewordsCount,
            int32_t forcedMask,
            OutputFormat outputFormat,
            bool isDebug)
{
    // Draw functional QR patterns
    qr->size = 4 * qr->version + 21;

    draw_functional_patterns(qr);

    if (isDebug) {
        fprintf(stderr, ">>> PLACING FUNCTIONAL PATTERNS\n");
        qr_print(stderr, qr, outputFormat);
        fprintf(stderr, "\n");
    }

    // Reserve format & version modules
    reserve_format_modules(qr);
    reserve_version_modules(qr);

    if (isDebug) {
        fprintf(stderr, ">>> RESERVING FORMAT & VERSION MODULES\n");
        qr_print(stderr, qr, outputFormat);
        fprintf(stderr, "\n");
    }

    // Draw QR data
    draw_data(qr, codewords, codewordsCount);

    if (isDebug) {
        fprintf(stderr, ">>> PLACING DATA MODULES\n");
        qr_print(stderr, qr, outputFormat);
        fprintf(stderr, "\n");
    }

    // Apply data masking
    int32_t minPenalty = INT32_MAX;
    int32_t bestMask = 0;
    if (forcedMask == MASK_INVALID) {
        for (int32_t mask = 0; mask < MASK_COUNT; mask++) {
            draw_format_modules(qr, mask);
            draw_version_modules(qr);
            apply_mask(qr, mask);

            int32_t penalty = calc_mask_penalty(qr);
            if (penalty < minPenalty) {
                minPenalty = penalty;
                bestMask = mask;
            }

            if (isDebug) {
                fprintf(stderr, ">>> PENALTY FOR MASK %d: %d\n", mask, penalty);
                qr_print(stderr, qr, outputFormat);
                fprintf(stderr, "\n");
            }

            // re-applying reverts the mask
            apply_mask(qr, mask);
        }
    }
    else {
        bestMask = forcedMask;
    }
    apply_mask(qr, bestMask);

    if (isDebug) {
        fprintf(stderr, ">>> APPLYING DATA MASK %d\n", bestMask);
        qr_print(stderr, qr, outputFormat);
        fprintf(stderr, "\n");
    }

    // Draw QR format and version
    draw_format_modules(qr, bestMask);
    draw_version_modules(qr);

    if (isDebug) {
        fprintf(stderr, ">>> PLACING FORMAT & VERSION MODULES\n");
        qr_print(stderr, qr, outputFormat);
        fprintf(stderr, "\n");
    }
}

QR
qr_encode(QROptions *options)
{
    char *text = options->text;
    int32_t textLen = options->textLen;
    ErrorCorrectionLevel forcedLevel = options->forcedLevel;
    int32_t forcedVersion = options->forcedVersion;
    bool isDebug = options->isDebug;

    QR qr = analyse_data(text, textLen, forcedLevel, forcedVersion, isDebug);

    BitVec bv = encode_data(&qr, text, textLen, isDebug);

    uint8_t codewords[MAX_BLOCKS_COUNT * MAX_BLOCKS_LENGTH] = {};
    int32_t codewordsCount = prepare_codewords(&qr, &bv, codewords, isDebug);

    int32_t forcedMask = options->forcedMask;
    OutputFormat outputFormat = options->outputFormat;
    draw_matrix(&qr, codewords, codewordsCount, forcedMask, outputFormat, isDebug);

    return qr;
}

internal void
print_ansi(FILE *out, QR *qr)
{
    for (int32_t row = -4; row < qr->size + 4; row++) {
        for (int32_t column = -4; column < qr->size + 4; column++) {
            if (row < 0 || row >= qr->size || column < 0 || column >= qr->size) {
                fprintf(out, "\033[47m  \033[0m"); // printing frame
                continue;
            }
            if (QR_MODULE_COLOR(qr, row, column) == MC_WHITE) {
                fprintf(out, "\033[47m  \033[0m");
            }
            else {
                fprintf(out, "\033[40m  \033[0m");
            }
        }
        fprintf(out, "\n");
    }
}

internal void
print_ascii(FILE *out, QR *qr)
{
    for (int32_t row = -4; row < qr->size + 4; row++) {
        for (int32_t column = -4; column < qr->size + 4; column++) {
            if (row < 0 || row >= qr->size || column < 0 || column >= qr->size) {
                fprintf(out, "  "); // printing frame
                continue;
            }
            if (QR_MODULE_COLOR(qr, row, column) == MC_WHITE) {
                fprintf(out, "  ");
            }
            else {
                fprintf(out, "##");
            }
        }
        fprintf(out, "\n");
    }
}

internal void
print_utf8(FILE *out, QR *qr) {
    for (int32_t row = -4; row < qr->size + 4; row += 2) {
        for (int32_t column = -4; column < qr->size + 4; column++) {
            ModuleColor color0;
            ModuleColor color1;
            if (row < 0 || row >= qr->size || column < 0 || column >= qr->size) {
                color0 = MC_WHITE;
                color1 = MC_WHITE;
            }
            else {
                color0 = QR_MODULE_COLOR(qr, row, column);
                color1 = (row == qr->size - 1) ? MC_WHITE : QR_MODULE_COLOR(qr, row + 1, column);
            }
            int32_t colors = color1 << 1 | color0;
            switch (colors) {
                case 0b00: fprintf(out, "%s", "█"); break;
                case 0b01: fprintf(out, "%s", "▄"); break;
                case 0b10: fprintf(out, "%s", "▀"); break;
                case 0b11: fprintf(out, "%s", " "); break;
                default: UNREACHABLE();
            }
        }
        fprintf(out, "\n");
    }
}

internal void
print_utf8q(FILE *out, QR *qr) {
    for (int32_t row = -4; row < qr->size + 4; row += 2) {
        for (int32_t column = -4; column < qr->size + 4; column += 2) {
            ModuleColor color0;
            ModuleColor color1;
            ModuleColor color2;
            ModuleColor color3;
            if (row < 0 || row >= qr->size || column < 0 || column >= qr->size) {
                color0 = MC_WHITE;
                color1 = MC_WHITE;
                color2 = MC_WHITE;
                color3 = MC_WHITE;
            }
            else {
                color0 = QR_MODULE_COLOR(qr, row, column);
                color1 = (column == qr->size - 1) ? MC_WHITE : QR_MODULE_COLOR(qr, row, column + 1);
                color2 = (row == qr->size - 1) ? MC_WHITE : QR_MODULE_COLOR(qr, row + 1, column);
                color3 = (row == qr->size - 1) || (column == qr->size - 1) ? MC_WHITE
                                                                           : QR_MODULE_COLOR(qr, row + 1, column + 1);
            }
            int32_t colors = color3 << 3 | color2 << 2 | color1 << 1 | color0;
            switch (colors) {
                case 0b0000: fprintf(out, "%s", "█"); break;
                case 0b0001: fprintf(out, "%s", "▟"); break;
                case 0b0010: fprintf(out, "%s", "▙"); break;
                case 0b0011: fprintf(out, "%s", "▄"); break;
                case 0b0100: fprintf(out, "%s", "▜"); break;
                case 0b0101: fprintf(out, "%s", "▐"); break;
                case 0b0110: fprintf(out, "%s", "▚"); break;
                case 0b0111: fprintf(out, "%s", "▗"); break;
                case 0b1000: fprintf(out, "%s", "▛"); break;
                case 0b1001: fprintf(out, "%s", "▞"); break;
                case 0b1010: fprintf(out, "%s", "▌"); break;
                case 0b1011: fprintf(out, "%s", "▖"); break;
                case 0b1100: fprintf(out, "%s", "▀"); break;
                case 0b1101: fprintf(out, "%s", "▝"); break;
                case 0b1110: fprintf(out, "%s", "▘"); break;
                case 0b1111: fprintf(out, "%s", " "); break;
                default: UNREACHABLE();
            }
        }
        fprintf(out, "\n");
    }
}

void
qr_print(FILE *out, QR *qr, OutputFormat outputFormat)
{
    switch (outputFormat) {
        case OF_ANSI: print_ansi(out, qr); break;
        case OF_ASCII: print_ascii(out, qr); break;
        case OF_UTF8: print_utf8(out, qr); break;
        case OF_UTF8Q: print_utf8q(out, qr); break;
        default: UNREACHABLE();
    }
}
