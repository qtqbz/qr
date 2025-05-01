#if BUILD_DEBUG
#define ASSERT(x)               \
    do {                        \
        if (!(x)) {             \
            __builtin_trap();   \
        }                       \
    } while (0)
#else
#define ASSERT(x) (void)(x)
#endif

#define ARRAY_CAP(a) ((int32_t)(sizeof(a) / sizeof((a)[0])))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
