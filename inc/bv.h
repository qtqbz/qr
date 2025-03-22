#ifndef BV_H
#define BV_H

#include <stdint.h>

#define MaxBitsCount 30000

typedef struct BitVec BitVec;
struct BitVec
{
    uint8_t bytes[MaxBitsCount / 8];
    int32_t size;
};

void bv_print(FILE *out, BitVec bv);
void bv_append(BitVec *bv,
               uint64_t bits,
               int32_t bitsCount);

#endif //BV_H
