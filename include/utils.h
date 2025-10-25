#ifndef UTILS_H
    #define UTILS_H
    #if defined(__GNUC__) || defined(__clang__)
        #define likely(x)   __builtin_expect(!!(x), 1)
        #define unlikely(x) __builtin_expect(!!(x), 0)
    #else
        #define likely(x)   (x)
        #define unlikely(x) (x)
    #endif
    #define UNUSED __attribute((unused))

#endif
