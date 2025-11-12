/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Orchestrates multiple playback rules in priority order to
 *               control message flow, timing, and delivery during replay.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include "playback_rules/playback_rule.h"
#include "playback_state.h"

#include <algorithm>
#include <memory>
#include <vector>

class RulesEngine {
public:
    RulesEngine() = default;
    
    // Add a rule to the engine (will be sorted by priority)
    void addRule(std::unique_ptr<PlaybackRule> rule) {
        rule->initialize();
        _rules.push_back(std::move(rule));
        sortRules();
    }
    
    // Evaluate all rules for a message
    PlaybackRule::Decision evaluate(size_t messageIndex,
                                    const char* message,
                                    const PlaybackState& state) {
        PlaybackRule::Decision decision;
        decision.outcome = PlaybackRule::Outcome::CONTINUE;
        decision.accumulatedDelay = std::chrono::microseconds(0);
        
        // Apply rules in priority order (SAFETY first, CHAOS last)
        for (auto& rule : _rules) {
            decision = rule->apply(messageIndex, message, state, decision);
            
            // Terminal outcomes short-circuit remaining rules
            if (decision.outcome == PlaybackRule::Outcome::VETO ||
                decision.outcome == PlaybackRule::Outcome::DROP) {
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
            [](const std::unique_ptr<PlaybackRule>& a,
               const std::unique_ptr<PlaybackRule>& b) {
                return a->getPriority() < b->getPriority();
            });
    }
    
    std::vector<std::unique_ptr<PlaybackRule>> _rules;
};
