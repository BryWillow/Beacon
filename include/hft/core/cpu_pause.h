// ---------------------------------------------------------------------------
// @file        cpu_pause.h
// Project     : Beacon
// Component   : HFT Core
// Description : Cross-platform CPU pause / spin hint for busy-wait loops
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

namespace beacon::hft::core
{

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
    /**
     * @brief Hint to the CPU that we are in a busy-wait spin loop.
     *
     * Improves power efficiency and reduces contention on x86/x64 CPUs.
     */
    inline void cpu_pause() { _mm_pause(); }

#else
    /**
     * @brief No-op fallback for non-x86 architectures.
     */
    inline void cpu_pause() { /* intentionally empty */ }
#endif

} // namespace beacon::hft::core
