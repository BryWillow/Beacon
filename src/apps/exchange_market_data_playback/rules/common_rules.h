/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Concrete implementations of common playback rules including
 *               burst patterns, rate limiting, speed control, and chaos testing.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <cmath>
#include <random>

#include "../playback_rule.h"
#include "../playback_state.h"

// =============================================================================
// CONTROL RULES (Priority::CONTROL)
// =============================================================================

/**
 * @brief Burst pattern rule - sends messages in bursts with pauses between
 */
class BurstRule : public PlaybackRule {
public:
    BurstRule(size_t burstSize, std::chrono::milliseconds burstInterval)
        : _burstSize(burstSize), _burstInterval(burstInterval) {}
    
    Priority getPriority() const override { return Priority::CONTROL; }
    
    void onPlaybackStart() override {
        _lastBurstTime = std::chrono::steady_clock::now();
        _messagesInCurrentBurst = 0;
    }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        if (_messagesInCurrentBurst < _burstSize) {
            // Within burst window - send immediately
            _messagesInCurrentBurst++;
            current.outcome = Outcome::SEND_NOW;
            current.accumulatedDelay = std::chrono::microseconds(0);
            return current;
        }
        
        // Burst complete - check if enough time has passed for next burst
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - _lastBurstTime);
        
        if (elapsed >= _burstInterval) {
            // Start new burst
            _messagesInCurrentBurst = 1;
            _lastBurstTime = now;
            current.outcome = Outcome::SEND_NOW;
            current.accumulatedDelay = std::chrono::microseconds(0);
            return current;
        }
        
        // Still waiting - add delay until next burst window
        auto waitTime = _burstInterval - elapsed;
        current.accumulatedDelay += std::chrono::duration_cast<std::chrono::microseconds>(waitTime);
        current.outcome = Outcome::MODIFIED;
        return current;
    }
    
private:
    size_t _burstSize;
    std::chrono::milliseconds _burstInterval;
    size_t _messagesInCurrentBurst = 0;
    std::chrono::steady_clock::time_point _lastBurstTime;
};

/**
 * @brief Continuous playback at fixed rate
 */
class ContinuousRule : public PlaybackRule {
public:
    ContinuousRule(size_t messagesPerSecond)
        : _messagesPerSecond(messagesPerSecond),
          _delayBetweenMessages(std::chrono::microseconds(
              static_cast<long>(1'000'000.0 / messagesPerSecond))) {}
    
    Priority getPriority() const override { return Priority::CONTROL; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        current.accumulatedDelay += _delayBetweenMessages;
        current.outcome = Outcome::MODIFIED;
        return current;
    }
    
private:
    size_t _messagesPerSecond;
    std::chrono::microseconds _delayBetweenMessages;
};

// =============================================================================
// TIMING RULES (Priority::TIMING)
// =============================================================================

/**
 * @brief Speed factor rule - scales all delays by a multiplier
 */
class SpeedFactorRule : public PlaybackRule {
public:
    SpeedFactorRule(double speedFactor) : _speedFactor(speedFactor) {}
    
    Priority getPriority() const override { return Priority::TIMING; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        // Scale any accumulated delay by speed factor
        if (current.accumulatedDelay.count() > 0) {
            current.accumulatedDelay = std::chrono::duration_cast<std::chrono::microseconds>(
                current.accumulatedDelay / _speedFactor);
            current.outcome = Outcome::MODIFIED;
        }
        return current;
    }
    
private:
    double _speedFactor;
};

/**
 * @brief Wave pattern rule - message rate oscillates smoothly
 */
class WaveRule : public PlaybackRule {
public:
    WaveRule(std::chrono::milliseconds period, size_t minRate, size_t maxRate)
        : _period(period), _minRate(minRate), _maxRate(maxRate) {}
    
    Priority getPriority() const override { return Priority::TIMING; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        // Calculate current rate based on sine wave
        double elapsed = state.elapsedMilliseconds();
        double phase = (elapsed / _period.count()) * 2.0 * M_PI;
        double rate = _minRate + (_maxRate - _minRate) * (std::sin(phase) + 1.0) / 2.0;
        
        auto delay = std::chrono::microseconds(static_cast<long>(1'000'000.0 / rate));
        current.accumulatedDelay += delay;
        current.outcome = Outcome::MODIFIED;
        return current;
    }
    
private:
    std::chrono::milliseconds _period;
    size_t _minRate;
    size_t _maxRate;
};

// =============================================================================
// SAFETY RULES (Priority::SAFETY)
// =============================================================================

/**
 * @brief Rate limiter - enforces maximum message rate (safety ceiling)
 */
class RateLimitRule : public PlaybackRule {
public:
    RateLimitRule(size_t maxMessagesPerSecond)
        : _maxRate(maxMessagesPerSecond) {}
    
    Priority getPriority() const override { return Priority::SAFETY; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        size_t currentRate = state.getCurrentRate();
        
        if (currentRate >= _maxRate) {
            // Exceeded rate limit - add backoff delay
            auto backoffDelay = std::chrono::microseconds(1'000'000 / _maxRate);
            current.accumulatedDelay += backoffDelay;
            current.outcome = Outcome::MODIFIED;
        }
        
        return current;
    }
    
private:
    size_t _maxRate;
};

// =============================================================================
// CHAOS RULES (Priority::CHAOS)
// =============================================================================

/**
 * @brief Packet loss simulation - randomly drops messages
 */
class PacketLossRule : public PlaybackRule {
public:
    PacketLossRule(double lossRate)
        : _lossRate(lossRate),
          _rng(std::random_device{}()),
          _dist(0.0, 1.0) {}
    
    Priority getPriority() const override { return Priority::CHAOS; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        // Don't drop if already vetoed by higher priority rule
        if (current.outcome == Outcome::VETO) {
            return current;
        }
        
        if (_dist(_rng) < _lossRate) {
            current.outcome = Outcome::DROP;
        }
        return current;
    }
    
private:
    double _lossRate;
    mutable std::mt19937 _rng;
    mutable std::uniform_real_distribution<double> _dist;
};

/**
 * @brief Jitter rule - adds random timing variance
 */
class JitterRule : public PlaybackRule {
public:
    JitterRule(std::chrono::microseconds maxJitter)
        : _maxJitter(maxJitter),
          _rng(std::random_device{}()),
          _dist(0, maxJitter.count()) {}
    
    Priority getPriority() const override { return Priority::CHAOS; }
    
    Decision apply(size_t idx [[maybe_unused]], const char* msg [[maybe_unused]],
                  const PlaybackState& state [[maybe_unused]], Decision current) override {
        auto jitter = std::chrono::microseconds(_dist(_rng));
        current.accumulatedDelay += jitter;
        current.outcome = Outcome::MODIFIED;
        return current;
    }
    
private:
    std::chrono::microseconds _maxJitter;
    mutable std::mt19937 _rng;
    mutable std::uniform_int_distribution<long> _dist;
};
