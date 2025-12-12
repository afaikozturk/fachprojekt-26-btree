#pragma once

#include <chrono>

#if defined(__x86_64__)
#include <immintrin.h>
#endif

class builtin
{
public:
    static inline void pause()
    {
#if defined(__x86_64__)
        _mm_pause();
#elif defined(__aarch64__)
        asm volatile("yield");
#endif
    }

template <class V> static inline __attribute__((always_inline)) void DoNotOptimize(V &value)
    {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }
};
