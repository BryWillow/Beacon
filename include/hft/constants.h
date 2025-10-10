// ---------------------------------------------------------------------------
// @file        constants.h
// Project     : Beacon
// Component   : Core
// Description : Common constants used throughout the Beacon HFT core
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

namespace beacon::hft
{

    /** @brief Indicates that no CPU pinning was requested for a thread */
    inline constexpr int NO_CPU_PINNING = -1;

    /** @brief Default capacity for single-producer single-consumer ring buffers */
    inline constexpr int DEFAULT_RING_BUFFER_CAPACITY = 1024;

} // namespace beacon::hft::core
