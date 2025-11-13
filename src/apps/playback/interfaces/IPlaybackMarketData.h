/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Abstract interface for message senders (UDP multicast, TCP,
 *               file output, etc.) used during market data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstddef>

namespace playback::replayer {
  class IPlaybackMarketData {
    public:
      virtual ~IPlaybackMarketData() = default;
      virtual bool send(const char* message, size_t length) = 0;
      virtual void flush() {}
      virtual size_t getMessagesSent() const = 0;
  };
} // namespace playback::replayer