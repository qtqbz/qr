#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#if BUILD_DEBUG
#define Assert(x)               \
    do {                        \
        if (!(x)) {             \
            __builtin_trap();   \
        }                       \
    } while (0)
#else
#define Assert(x) (void)(x)
#endif

#define ArrayCap(a) ((int32_t)(sizeof(a) / sizeof((a)[0])))

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#endif //UTILS_H
