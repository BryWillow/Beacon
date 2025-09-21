#pragma once
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#define _mm_pause() _mm_pause()
#else
#define _mm_pause() do {} while(0)
#endif
