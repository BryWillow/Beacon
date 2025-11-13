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

namespace playback::advisors {

  class PlaybackState; // Forward declaration

  enum class MessagePriority {
    NORMAL = 0,
    ELEVATED = 1,
    CRITICAL = 2,
    EMERGENCY = 3
  };

  class IMessagePriorityClassifier {
    public:
      virtual ~IMessagePriorityClassifier() = default;
      virtual MessagePriority classify(size_t messageIndex,
                                      const char* message,
                                      const PlaybackState& state) = 0;
  };
} // namespace playback::advisors