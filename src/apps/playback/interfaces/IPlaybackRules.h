/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Abstract interface for playback rules (e.g., time-based,
 *               sequence-based) used during market data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <cstddef>

namespace playback::authorities {

class PlaybackState; // Forward declaration

class IPlaybackRule {
  public:
    enum class Priority {
      SAFETY = 0,
      CONTROL = 1,
      TIMING = 2,
      CHAOS = 3
    };

    enum class Outcome {
      CONTINUE,
      SEND_NOW,
      DROP,
      VETO,
      MODIFIED
    };

    struct Decision {
      Outcome outcome = Outcome::CONTINUE;
      std::chrono::microseconds accumulatedDelay{0};
      void* metadata = nullptr;
    };

  virtual ~IPlaybackRule() = default;
  virtual Priority getPriority() const = 0;
  virtual Decision apply(size_t messageIndex,
                        const char* message,
                        const PlaybackState& state,
                        Decision currentDecision) = 0;
  virtual void initialize() {}
  virtual void onPlaybackStart() {}
  virtual void onPlaybackEnd() {}
};

} // namespace playback::authorities