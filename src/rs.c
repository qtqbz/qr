#include "rs.h"
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
                  int32_t polyDegree,
                  const uint8_t *divisor,
                  int32_t divisorDegree,
                  uint8_t *remainder)
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
