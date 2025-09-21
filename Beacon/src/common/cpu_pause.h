// ---------------------------------------------------------------------------
// File        : cpu_pause.h
// Project     : Beacon
// Component   : Common
// Description : Cross-platform CPU pause stub for HFT busy-wait loops
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

/**
 * @file cpu_pause.h
 * @brief Provides a cross-platform stub for _mm_pause / CPU pause instruction.
 *
 * Purpose:
 * - Reduces busy-wait CPU pressure in HFT hot-paths.
 * - On x86_64, uses real `_mm_pause` from SSE2 intrinsics.
 * - On other platforms (ARM / Apple Silicon), defines an empty inline stub.
 */
#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h> ///< SSE2 intrinsics for _mm_pause
#else
inline void _mm_pause() {} ///< stub for ARM / Apple Silicon
#endif
