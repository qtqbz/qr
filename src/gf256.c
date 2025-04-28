#include "gf256.h"
#include "utils.h"

uint8_t
gf256_multiply(uint8_t a, uint8_t b)
{
    return (a && b) ? Exp[(Log[a] + Log[b]) % 255] : 0;
}

uint8_t
gf256_divide(uint8_t a, uint8_t b)
{
    Assert(b != 0);
    return Exp[(Log[a] + Log[b] * 254) % 255];
}

int32_t
gf256_poly_divide(const uint8_t *poly,
                  int32_t polyLength,
                  const uint8_t *divisor,
                  int32_t divisorLength,
                  uint8_t *remainder)
{
    for (int32_t i = 0; i < polyLength; i++) {
        remainder[i] = poly[i];
    }

    int32_t index = 0;
    for (int32_t i = divisorLength - 1; i < polyLength; i++) {
        if (remainder[index]) {
            uint8_t factor = gf256_divide(remainder[index], divisor[0]);
            for (int32_t j = 0; j < divisorLength; j++) {
                remainder[index + j] ^= gf256_multiply(factor, divisor[j]);
            }
        }
        index++;
    }

    while (index < polyLength && remainder[index] == 0) {
        index++;
    }

    int32_t remainderLength = polyLength - index;
    for (int32_t i = index; i < polyLength; i++) {
        remainder[i - index] = remainder[i];
    }

    return remainderLength;
}
