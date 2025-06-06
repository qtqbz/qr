#include "utils.h"
#include "bv.h"

void
bv_print_bin(FILE *out, BitVec bv)
{
    for (int32_t i = 0; i < bv.size; i++) {
        fprintf(out, "%c%s", ((bv.bytes[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0', (((i + 1) % 8) == 0) ? " " : "");
    }
    fprintf(out, "\n");
}

void
bv_print_hex(FILE *out, BitVec bv)
{
    for (int32_t i = 0; i < bv.size; i += 8) {
        fprintf(out, "%02X ", bv.bytes[i / 8]);
    }
    fprintf(out, "\n");
}

void
bv_append(BitVec *bv, uint64_t bits, int32_t bitsCount)
{
    ASSERT(0 <= bitsCount && bitsCount <= 64);
    ASSERT((bitsCount == 64) || ((bits >> bitsCount) == 0));
    ASSERT((bv->size + bitsCount) <= (ARRAY_CAP(bv->bytes) * 8));

    while (bitsCount--) {
        bv->bytes[bv->size / 8] |= ((bits >> bitsCount) & 1) << (7 - (bv->size % 8));
        bv->size++;
    }
}
