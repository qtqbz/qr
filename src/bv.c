#include <stdio.h>

#include "bv.h"
#include "utils.h"

void
bv_print(FILE *out, BitVec bv)
{
    for (int32_t i = 0; i < bv.size; i++) {
        fprintf(out, "%c%s", ((bv.bytes[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0', (((i + 1) % 8) == 0) ? " " : "");
    }
    fprintf(out, "\n");
}

void
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
