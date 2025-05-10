#ifndef QR_H
#define QR_H

#include <stdint.h>
#include <stdio.h>

#define MIN_VERSION 0
#define MAX_VERSION 39
#define VERSION_COUNT 40
#define VERSION_INVALID (-1)

#define LEVEL_INVALID (-1)

#define MIN_MASK 0
#define MAX_MASK 7
#define MASK_COUNT 8
#define MASK_INVALID (-1)

#define MAX_TEXT_LEN 7089 // = MAX_CHAR_COUNT[EM_NUMERIC][ECL_LOW][MAX_VERSION]

#define MAX_MODULE_COUNT 31329 // = (4 × MaxVersion + 21) * (4 × MaxVersion + 21)

typedef int32_t EncodingMode;
enum EncodingMode
{
    EM_NUMERIC,
    EM_ALPHANUM,
    EM_BYTE,
    //EM_KANJI,

    EM_COUNT,
};

global const char *const EncodingModeNames[EM_COUNT] = {
    "NUMERIC",
    "ALPHANUMERIC",
    "BYTE",
    //"KANJI",
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

global const char *const ErrorCorrectionLevelNames[ECL_COUNT] = {
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

typedef int32_t OutputFormat;
enum OutputFormat
{
    OF_ANSI,
    OF_ASCII,
    OF_UTF8,
    OF_UTF8Q,
};

typedef struct QROptions QROptions;
struct QROptions
{
    char text[MAX_TEXT_LEN + 1];
    int32_t textLen;
    ErrorCorrectionLevel forcedLevel;
    int32_t forcedVersion;
    int32_t forcedMask;
    OutputFormat outputFormat;
    bool isDebug;
};

typedef struct QR QR;
struct QR
{
    ModuleValue matrix[MAX_MODULE_COUNT];
    int32_t size;
    EncodingMode mode;
    ErrorCorrectionLevel level;
    int32_t version;
};

QR qr_encode(QROptions *options);
void qr_print(FILE *out, QR *qr, OutputFormat outputFormat);

#endif //QR_H
