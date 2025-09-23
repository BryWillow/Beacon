// ---------------------------------------------------------------------------
// File        : md_itch_captured_message.h
// Project     : Beacon
// Component   : Utilities / Market Data Capture
// Description : Wrapper for ITCH messages with a capture timestamp
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include "md_itch_message.h"

/**
 * @struct CapturedItchMessage
 * @brief Combines a single ITCH message with its capture timestamp.
 *
 * Notes:
 * - Used for storing messages in memory or writing to disk.
 * - Ensures a consistent format across generator, listener, and replayer.
 */
struct CapturedItchMessage {
    ItchMessage msg;   ///< The actual ITCH message
    uint64_t    tsNs;  ///< Timestamp (nanoseconds) when the message was captured
};
