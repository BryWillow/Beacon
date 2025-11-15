/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Abstract interface for message priority classification.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "../playback_state.h"

namespace playback::advisors {

  enum class MessagePriority {
    NORMAL = 0,
    ELEVATED = 1,
    CRITICAL = 2,
    EMERGENCY = 3
  };

  class IClassifyMessagePriority {
  public:
    virtual ~IClassifyMessagePriority() = default;
    virtual MessagePriority classify(size_t messageIndex,
                                    const char* message,
                                    const PlaybackState& state) = 0;
  };

} // namespace playback::advisors