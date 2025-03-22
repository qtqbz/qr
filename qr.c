#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define internal      static
#define global        static
#define local_persist static

#if BUILD_DEBUG
#define Assert(x)               \
    do {                        \
        if (!(x)) {             \
            __builtin_trap();   \
        }                       \
    } while (0)
#else
#define Assert(x) (void)(x)
#endif

#define ArrayCap(a) ((int32_t)(sizeof(a) / sizeof((a)[0])))

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))


// ==== BIT VECTOR ====

#define MaxBitsCount 30000

typedef struct BitVec BitVec;
struct BitVec
{
    uint8_t bytes[MaxBitsCount / 8];
    int32_t size;
};

internal void bv_print(BitVec bv);
internal void bv_append(BitVec *bv, uint64_t bits, int32_t bitsCount);

internal void
bv_print(BitVec bv)
{
    for (int32_t i = 0; i < bv.size; i++) {
        printf("%c%s", ((bv.bytes[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0', (((i + 1) % 8) == 0) ? " " : "");
    }
    printf("\n");
}

internal void
bv_append(BitVec *bv, uint64_t bits, int32_t bitsCount)
{
    Assert(0 <= bitsCount && bitsCount <= 64);
    Assert((bitsCount == 64) || ((bits >> bitsCount) == 0));
    Assert((bv->size + bitsCount) <= ArrayCap(bv->bytes));

    while (bitsCount--) {
        bv->bytes[bv->size / 8] |= ((bits >> bitsCount) & 1) << (7 - (bv->size % 8));
        bv->size++;
    }
}


// ==== REED-SOLOMON ERROR CORRECTION ====

#define MaxPolygonDegree 123 // = max data codewords per block
#define MaxGenPolygonDegree 30 // = max error correction codewords per block

global const uint8_t Exp[256] = {
      1,   2,   4,   8,  16,  32,  64, 128,  29,  58, 116, 232, 205, 135,  19,  38,
     76, 152,  45,  90, 180, 117, 234, 201, 143,   3,   6,  12,  24,  48,  96, 192,
    157,  39,  78, 156,  37,  74, 148,  53, 106, 212, 181, 119, 238, 193, 159,  35,
     70, 140,   5,  10,  20,  40,  80, 160,  93, 186, 105, 210, 185, 111, 222, 161,
     95, 190,  97, 194, 153,  47,  94, 188, 101, 202, 137,  15,  30,  60, 120, 240,
    253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163,  91, 182, 113, 226,
    217, 175,  67, 134,  17,  34,  68, 136,  13,  26,  52, 104, 208, 189, 103, 206,
    129,  31,  62, 124, 248, 237, 199, 147,  59, 118, 236, 197, 151,  51, 102, 204,
    133,  23,  46,  92, 184, 109, 218, 169,  79, 158,  33,  66, 132,  21,  42,  84,
    168,  77, 154,  41,  82, 164,  85, 170,  73, 146,  57, 114, 228, 213, 183, 115,
    230, 209, 191,  99, 198, 145,  63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
    227, 219, 171,  75, 150,  49,  98, 196, 149,  55, 110, 220, 165,  87, 174,  65,
    130,  25,  50, 100, 200, 141,   7,  14,  28,  56, 112, 224, 221, 167,  83, 166,
     81, 162,  89, 178, 121, 242, 249, 239, 195, 155,  43,  86, 172,  69, 138,   9,
     18,  36,  72, 144,  61, 122, 244, 245, 247, 243, 251, 235, 203, 139,  11,  22,
     44,  88, 176, 125, 250, 233, 207, 131,  27,  54, 108, 216, 173,  71, 142,   1,
};

global const uint8_t Log[256] = {
      0,   0,   1,  25,   2,  50,  26, 198,   3, 223,  51, 238,  27, 104, 199,  75,
      4, 100, 224,  14,  52, 141, 239, 129,  28, 193, 105, 248, 200,   8,  76, 113,
      5, 138, 101,  47, 225,  36,  15,  33,  53, 147, 142, 218, 240,  18, 130,  69,
     29, 181, 194, 125, 106,  39, 249, 185, 201, 154,   9, 120,  77, 228, 114, 166,
      6, 191, 139,  98, 102, 221,  48, 253, 226, 152,  37, 179,  16, 145,  34, 136,
     54, 208, 148, 206, 143, 150, 219, 189, 241, 210,  19,  92, 131,  56,  70,  64,
     30,  66, 182, 163, 195,  72, 126, 110, 107,  58,  40,  84, 250, 133, 186,  61,
    202,  94, 155, 159,  10,  21, 121,  43,  78, 212, 229, 172, 115, 243, 167,  87,
      7, 112, 192, 247, 140, 128,  99,  13, 103,  74, 222, 237,  49, 197, 254,  24,
    227, 165, 153, 119,  38, 184, 180, 124,  17,  68, 146, 217,  35,  32, 137,  46,
     55,  63, 209,  91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190,  97,
    242,  86, 211, 171,  20,  42,  93, 158, 132,  60,  57,  83,  71, 109,  65, 162,
     31,  45,  67, 216, 183, 123, 164, 118, 196,  23,  73, 236, 127,  12, 111, 246,
    108, 161,  59,  82,  41, 157,  85, 170, 251,  96, 134, 177, 187, 204,  62,  90,
    203,  89,  95, 176, 156, 169, 160,  81,  11, 245,  22, 235, 122, 117,  44, 215,
     79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168,  80,  88, 175,
};

global const uint8_t GenPoly[MaxGenPolygonDegree + 1][MaxGenPolygonDegree + 1] = {
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

internal uint8_t gf256_multiply(uint8_t a, uint8_t b);
internal uint8_t gf256_divide(uint8_t a, uint8_t b);
internal int32_t gf256_poly_divide(const uint8_t *poly, int32_t polyDegree, const uint8_t *divisor, int32_t divisorDegree, uint8_t *remainder);

internal uint8_t
gf256_multiply(uint8_t a, uint8_t b)
{
    return (a && b) ? Exp[(Log[a] + Log[b]) % 255] : 0;
}

internal uint8_t
gf256_divide(uint8_t a, uint8_t b)
{
    Assert(b != 0);
    return Exp[(Log[a] + Log[b] * 254) % 255];
}

internal int32_t
gf256_poly_divide(const uint8_t *poly, int32_t polyDegree, const uint8_t *divisor, int32_t divisorDegree, uint8_t *remainder)
{
    Assert(0 <= polyDegree && divisorDegree <= MaxPolygonDegree);
    Assert(0 <= divisorDegree && divisorDegree <= MaxGenPolygonDegree);
    Assert(divisor[0] != 0);

    for (int32_t i = 0; i <= polyDegree; i++) {
        remainder[i] = poly[i];
    }

    int32_t index = 0;
    for (int32_t i = divisorDegree; i <= polyDegree; i++) {
        uint8_t factor = gf256_divide(remainder[index], divisor[0]);
        for (int32_t j = 0; j <= divisorDegree; j++) {
            remainder[index + j] ^= gf256_multiply(factor, divisor[j]);
        }
        index++;
    }

    while (index <= polyDegree && remainder[index] == 0) {
        index++;
    }

    int32_t remainderDegree = polyDegree - index;
    for (int32_t i = index; i <= polyDegree; i++) {
        remainder[i - index] = remainder[i];
    }

    return remainderDegree;
}


// ==== QR ====

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

global const char *const EncodingModeNames[EncodingModeCount] = {
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

global const char *const ErrorCorrectionLevelNames[ErrorCorrectionLevelCount] = {
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
global const int32_t MaxCharCount[EncodingModeCount][ErrorCorrectionLevelCount][VersionCount] = {
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
global const int32_t LengthBitsCount[EncodingModeCount][VersionCount] = {
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
global const int32_t ContentModulesCount[VersionCount] = {
      208,   359,   567,   807,  1079,  1383,  1568,  1936,  2336,  2768,
     3232,  3728,  4256,  4651,  5243,  5867,  6523,  7211,  7931,  8683,
     9252, 10068, 10916, 11796, 12708, 13652, 14628, 15371, 16411, 17483,
    18587, 19723, 20891, 22091, 23008, 24272, 25568, 26896, 28256, 29648,
};

// Number of ECC blocks.
global const int32_t ErrorCorrectionBlocksCount[ErrorCorrectionLevelCount][VersionCount] = {
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
global const int32_t ErrorCorrectionCodewordsPerBlockCount[ErrorCorrectionLevelCount][VersionCount] = {
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

global const int32_t AlignmentCoordinates[VersionCount][AlignmentCoordinatesCount] = {
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

global const uint32_t FormatBits[ErrorCorrectionLevelCount][MaskCount] = {
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

internal int32_t qr_calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level);
internal int32_t qr_find_first_unmatch_index(const char *text, int32_t textCharCount, char *charsToMatch, int32_t offset);
internal EncodingMode qr_get_encoding_mode(const char *text, int32_t textCharCount);
internal int32_t qr_get_version(EncodingMode mode, ErrorCorrectionLevel level, int32_t textCharCount);

internal int32_t
qr_calc_data_codewords_count(int32_t version, ErrorCorrectionLevel level)
{
    int32_t contentCodewordsCount = ContentModulesCount[version] / 8;
    int32_t errorCorrectionCodewordsCount = ErrorCorrectionBlocksCount[level][version] * ErrorCorrectionCodewordsPerBlockCount[level][version];
    int32_t dataCodewordsCount = contentCodewordsCount - errorCorrectionCodewordsCount;
    return dataCodewordsCount;
}

internal int32_t
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

internal EncodingMode
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

internal int32_t
qr_get_version(EncodingMode mode, ErrorCorrectionLevel level, int32_t textCharCount)
{
    for (int32_t version = MinVersion; version <= MaxVersion; version++) {
        if (textCharCount <= MaxCharCount[mode][level][version]) {
            return version;
        }
    }
    return VersionInvalid;
}

internal void
qr_draw_module(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, ModuleFlag value)
{
    if ((0 <= row && row < qrSize) && (0 <= column && column < qrSize)) {
        qrCode[row * qrSize + column] = value;
    }
}

internal void
qr_draw_rectangle(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, int32_t width, int32_t height, ModuleFlag value)
{
    for (int32_t i = 0; i < height; i++) {
        for (int32_t j = 0; j < width; j++) {
            qr_draw_module(qrCode, qrSize, row + i, column + j, value);
        }
    }
}

internal void
qr_draw_square(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column, int32_t size, ModuleFlag value)
{
    qr_draw_rectangle(qrCode, qrSize, row, column, size, size, value);
}

internal void
qr_draw_finder_pattern(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column)
{
    qr_draw_square(qrCode, qrSize, row - 1, column - 1, 9, ModuleFunctional | ModuleWhite); // separator
    qr_draw_square(qrCode, qrSize, row + 0, column + 0, 7, ModuleFunctional | ModuleBlack);
    qr_draw_square(qrCode, qrSize, row + 1, column + 1, 5, ModuleFunctional | ModuleWhite);
    qr_draw_square(qrCode, qrSize, row + 2, column + 2, 3, ModuleFunctional | ModuleBlack);
}

internal void
qr_draw_finder_patterns(uint8_t *qrCode, int32_t qrSize)
{
    qr_draw_finder_pattern(qrCode, qrSize, 0, 0);
    qr_draw_finder_pattern(qrCode, qrSize, 0, qrSize - 7);
    qr_draw_finder_pattern(qrCode, qrSize, qrSize - 7, 0);
}

internal void
qr_draw_alignment_pattern(uint8_t *qrCode, int32_t qrSize, int32_t row, int32_t column)
{
    qr_draw_square(qrCode, qrSize, row + 0, column + 0, 5, ModuleFunctional | ModuleBlack);
    qr_draw_square(qrCode, qrSize, row + 1, column + 1, 3, ModuleFunctional | ModuleWhite);
    qr_draw_module(qrCode, qrSize, row + 2, column + 2, ModuleFunctional | ModuleBlack);
}

internal void
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

internal void
qr_draw_timing_patterns(uint8_t *qrCode, int32_t qrSize)
{
    ModuleFlag value = ModuleFunctional | ModuleBlack;
    for (int32_t i = 8; i < (qrSize - 7); i++) {
        qr_draw_module(qrCode, qrSize, 6, i, value);
        qr_draw_module(qrCode, qrSize, i, 6, value);
        value = value ^ ModuleBlack ^ ModuleWhite;
    }
}

internal void
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
                qrCode[row * qrSize + column] = ((codeword >> bitIndex) & 1) ? ModuleBlack : ModuleWhite;
                if (bitIndex == 0) {
                    codewordsIndex++;
                    bitIndex = 7;
                }
                else {
                    bitIndex--;
                }
            }
            else {
                qrCode[row * qrSize + column] = ModuleWhite;
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

internal void
qr_print(uint8_t *qrCode, int32_t qrSize)
{
    for (int32_t i = -1; i <= qrSize; i++) {
        for (int32_t j = -1; j <= qrSize; j++) {
            if (i < 0 || i == qrSize || j < 0 || j == qrSize) {
                // Drawing frame
                printf("\033[47m  \033[0m");
                continue;
            }

            uint8_t value = qrCode[i * qrSize + j];
            if (value & ModuleBlack) {
                printf("\033[40m  \033[0m");
            }
            else if (value & ModuleWhite) {
                printf("\033[47m  \033[0m");
            }
            else {
                // Empty module
                printf("\033[50m  \033[0m");
            }
        }
        printf("\n");
    }
}

internal void
qr_apply_mask(uint8_t *qrCode, int32_t qrSize, int32_t mask)
{
    for (int32_t row = 0; row < qrSize; row++) {
        for (int32_t column = 0; column < qrSize; column++) {
            uint8_t value = qrCode[row * qrSize + column];
            if ((value & ModuleFunctional) || (value & ModuleReserved)) {
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
                qrCode[row * qrSize + column] = value ^ ModuleBlack ^ ModuleWhite;
            }
        }
    }
}

internal void
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
        qr_draw_module(qrCode, qrSize, row, 8, ((formatBits >> bitIndex) & 1) ? ModuleBlack : ModuleWhite);
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
        qr_draw_module(qrCode, qrSize, 8, column, ((formatBits >> bitIndex) & 1) ? ModuleBlack : ModuleWhite);
        bitIndex--;
    }
}


// ==== MAIN ====

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
