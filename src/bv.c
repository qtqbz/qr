#define MAX_BITS_COUNT 30000

typedef struct BitVec BitVec;
struct BitVec
{
    uint8_t bytes[MAX_BITS_COUNT / 8];
    int32_t size;
};

static void
bv_print(FILE *out, BitVec bv)
{
    for (int32_t i = 0; i < bv.size; i++) {
        fprintf(out, "%c%s", ((bv.bytes[i / 8] >> (7 - (i % 8))) & 1) ? '1' : '0', (((i + 1) % 8) == 0) ? " " : "");
    }
    fprintf(out, "\n");
}

static void
bv_append(BitVec *bv, uint64_t bits, int32_t bitsCount)
{
    ASSERT(0 <= bitsCount && bitsCount <= 64);
    ASSERT((bitsCount == 64) || ((bits >> bitsCount) == 0));
    ASSERT((bv->size + bitsCount) <= ARRAY_CAP(bv->bytes));

    while (bitsCount--) {
        bv->bytes[bv->size / 8] |= ((bits >> bitsCount) & 1) << (7 - (bv->size % 8));
        bv->size++;
    }
}
