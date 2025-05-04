#ifndef GF256_H
#define GF256_H

#include <stdint.h>

uint8_t gf256_multiply(uint8_t a, uint8_t b);
uint8_t gf256_divide(uint8_t a, uint8_t b);
int32_t gf256_polynom_divide(uint8_t *poly,
                             int32_t polynomLen,
                             uint8_t *divisor,
                             int32_t divisorLen,
                             uint8_t *remainder);

#endif //GF256_H
