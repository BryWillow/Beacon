// ---------------------------------------------------------------------------
// File        : captured_message.h
// Project     : Beacon
// Component   : Common
// Description : Converts Itch messages to host and network ordering.
// Author      : Bryan Camp
// ---------------------------------------------------------------------------
#pragma once

#include "itch_message.h"
#include <cstdint>

/**
 * @file captured_message.h
 * @brief Stores a captured ITCH message along with its timestamp for replay.
 */
struct CapturedMessage {
    ItchMessage msg;   ///< The ITCH message itself
    uint64_t tsNs;     ///< Capture timestamp in nanoseconds

    /// @brief Convert network-order fields to host order
    void toHostOrder() {
        msg.toHostOrder();
        tsNs = ntohll(tsNs);
    }

    /// @brief Convert host-order fields back to network order
    void toNetworkOrder() {
        msg.toNetworkOrder();
        tsNs = htonll(tsNs);
    }
};