/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Rules engine for managing and applying playback rules.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <vector>
#include <memory>
#include "../interfaces/IPlaybackRules.h"
#include "playback_state.h"

namespace playback::playback_authorities {

// Use IPlaybackRule::Priority, IPlaybackRule::Outcome, IPlaybackRule::Decision directly
using Priority = IPlaybackRule::Priority;
using Outcome = IPlaybackRule::Outcome;
using Decision = IPlaybackRule::Decision;

class RulesEngine {
public:
    RulesEngine() = default;
    
    // Add a rule to the engine (will be sorted by priority)
    void addRule(std::unique_ptr<IPlaybackRule> rule) {
        rule->initialize();
        _rules.push_back(std::move(rule));
        sortRules();
    }
    
    // Evaluate all rules for a message
    Decision evaluate(size_t messageIndex,
                      const char* message,
                      const PlaybackState& state) {
        Decision decision;
        decision.outcome = Outcome::CONTINUE;
        decision.accumulatedDelay = std::chrono::microseconds(0);
        
        // Apply rules in priority order (SAFETY first, CHAOS last)
        for (auto& rule : _rules) {
            decision = rule->apply(messageIndex, message, state, decision);
            
            // Terminal outcomes short-circuit remaining rules
            if (decision.outcome == Outcome::VETO ||
                decision.outcome == Outcome::DROP) {
                break;  // Higher priority rule blocked sending
            }
        }
        
        return decision;
    }
    
    // Notify all rules that playback is starting
    void notifyPlaybackStart() {
        for (auto& rule : _rules) {
            rule->onPlaybackStart();
        }
    }
    
    // Notify all rules that playback has ended
    void notifyPlaybackEnd() {
        for (auto& rule : _rules) {
            rule->onPlaybackEnd();
        }
    }
    
    // Get count of registered rules
    size_t getRuleCount() const { return _rules.size(); }
    
private:
    void sortRules() {
        std::sort(_rules.begin(), _rules.end(),
            [](const std::unique_ptr<IPlaybackRule>& a,
               const std::unique_ptr<IPlaybackRule>& b) {
                return a->getPriority() < b->getPriority();
            });
    }
    
    std::vector<std::unique_ptr<IPlaybackRule>> _rules;
};

} // namespace playback::playback_authorities
