// ---------------------------------------------------------------------------
// File        : constants.h
// Project     : Beacon
// Component   : Common
// Description : Compile-time constants for HFT simulator
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstddef>

namespace hft {

// Default SPSC ring buffer size (must be power-of-2)
inline constexpr size_t DEFAULT_RING_BUFFER_CAPACITY = 4096;

// Special value to indicate no CPU pinning
inline constexpr int NO_CPU_PINNING = -1;

} // namespace hft
