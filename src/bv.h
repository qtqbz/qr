#ifndef BV_H
#define BV_H

#include <stdint.h>
#include <stdio.h>

#define MAX_BITS_COUNT 30000

typedef struct BitVec BitVec;
struct BitVec
{
    uint8_t bytes[MAX_BITS_COUNT / 8];
    int32_t size;
};

void bv_print_bin(FILE *out, BitVec bv);
void bv_print_hex(FILE *out, BitVec bv);
void bv_append(BitVec *bv, uint64_t bits, int32_t bitsCount);

#endif //BV_H
