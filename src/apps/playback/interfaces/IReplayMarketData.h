/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Abstract interface for message senders (UDP multicast, TCP,
 *               file output, etc.) used during market data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstddef>

/**
 * @brief Abstract interface for sending messages during playback
 */

namespace market_data_playback {
    class IPlaybackMarketData {
    public:
        virtual ~IPlaybackMarketData() = default;

        // Send a message (returns true on success)
        virtual bool send(const char* message, size_t length) = 0;

        // Flush any buffered messages
        virtual void flush() = 0;

        // Get statistics about messages sent
        virtual size_t getMessagesSent() const = 0;
    };
} // namespace market_data_playback