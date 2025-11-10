/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Abstract base class for playback rules that control message
 *               flow, timing, and delivery during market data replay.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <cstddef>

// Forward declaration
class PlaybackState;

class PlaybackRule {
public:
    // Priority levels: lower number = higher priority (evaluated first).
    enum class Priority {
        SAFETY = 0,      // Highest - never violated (rate limits, system protection)
        CONTROL = 1,     // High    - flow control (burst patterns, queuing)
        TIMING = 2,      // Medium  - timing adjustments (speed factor, delays)
        CHAOS = 3        // Lowest  - testing rules (packet loss, jitter)
    };
    
    // Outcomes that rules can produce.
    enum class Outcome {
        CONTINUE,        // Pass to next rule (default)
        SEND_NOW,        // Send immediately (clears delays)
        DROP,            // Drop message (terminal)
        VETO,            // Block sending - higher priority than DROP (terminal)
        MODIFIED         // Decision was modified, continue to next rule
    };
    
    // Decision structure passed between rules.
    struct Decision {
        Outcome outcome = Outcome::CONTINUE;
        std::chrono::microseconds accumulatedDelay{0};
        void* metadata = nullptr;  // For passing data between rules
    };
    
    virtual ~PlaybackRule() = default;
    
    // Get the priority level of this rule
    virtual Priority getPriority() const = 0;
    
    // Apply the rule to a message and update the decision
    // Returns the updated decision (may be same, modified, or completely changed)
    virtual Decision apply(size_t messageIndex,
                          const char* message,
                          const PlaybackState& state,
                          Decision currentDecision) = 0;
    
    // Optional: Called when rule is added to engine (for initialization)
    virtual void initialize() {}
    
    // Optional: Called at start of playback
    virtual void onPlaybackStart() {}
    
    // Optional: Called at end of playback
    virtual void onPlaybackEnd() {}
};
