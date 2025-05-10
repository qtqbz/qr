#ifndef UTILS_H
#define UTILS_H

#if BUILD_DEBUG
#define ASSERT(x)               \
    do {                        \
        if (!(x)) {             \
            __builtin_trap();   \
        }                       \
    } while (0)
#define UNREACHABLE() __builtin_trap()
#else
#define ASSERT(x) (void)(x)
#define UNREACHABLE() (void)(x)
#endif

#define ARRAY_CAP(a) ((int32_t)(sizeof(a) / sizeof((a)[0])))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif //UTILS_H
