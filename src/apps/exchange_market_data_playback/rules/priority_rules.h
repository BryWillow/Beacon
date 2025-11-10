/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Event priority system that allows important messages to bypass
 *               flow control and safety rules (e.g., market moves bypass rate limits)
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstring>
#include <unordered_set>

#include "../playback_rule.h"
#include "../playback_state.h"


/**
 * @brief Message priority levels - higher priority messages can bypass rules
 */
enum class MessagePriority {
    NORMAL = 0,      // Regular messages, subject to all rules
    ELEVATED = 1,    // Important messages, bypass chaos rules
    CRITICAL = 2,    // Critical messages, bypass flow control + chaos
    EMERGENCY = 3    // Emergency messages, bypass ALL rules (even safety)
};

/**
 * @brief Classifier that determines message priority based on content
 */
class MessagePriorityClassifier {
public:
    virtual ~MessagePriorityClassifier() = default;
    
    // Examine a message and assign priority level
    virtual MessagePriority classify(size_t messageIndex,
                                     const char* message,
                                     const PlaybackState& state) = 0;
};

/**
 * @brief Symbol-based classifier - certain symbols are high priority
 */
class SymbolPriorityClassifier : public MessagePriorityClassifier {
public:
    // Add a high-priority symbol (e.g., "SPY", "QQQ")
    void addCriticalSymbol(const std::string& symbol) {
        _criticalSymbols.insert(symbol);
    }
    
    MessagePriority classify(size_t idx, const char* msg, const PlaybackState& state) override {
        // Symbol is at offset 12 in our 32-byte messages
        char symbolBuf[9] = {0};
        std::memcpy(symbolBuf, msg + 12, 8);
        
        // Trim spaces
        std::string symbol(symbolBuf);
        size_t end = symbol.find_last_not_of(' ');
        if (end != std::string::npos) {
            symbol = symbol.substr(0, end + 1);
        }
        
        // Check if this is a critical symbol
        if (_criticalSymbols.count(symbol) > 0) {
            return MessagePriority::CRITICAL;
        }
        
        return MessagePriority::NORMAL;
    }
    
private:
    std::unordered_set<std::string> _criticalSymbols;
};

/**
 * @brief Rate-based classifier - detect bursts/market moves
 * If incoming messages suddenly spike, mark them as elevated priority
 */
class BurstDetectionClassifier : public MessagePriorityClassifier {
public:
    BurstDetectionClassifier(double burstThreshold = 2.0)
        : _burstThreshold(burstThreshold) {}
    
    MessagePriority classify(size_t idx, const char* msg, const PlaybackState& state) override {
        size_t currentRate = state.getCurrentRate();
        
        // Calculate average rate over time
        if (_averageRate == 0) {
            _averageRate = currentRate;
            return MessagePriority::NORMAL;
        }
        
        // Smooth average rate
        _averageRate = 0.9 * _averageRate + 0.1 * currentRate;
        
        // If current rate is significantly above average, this is a market move
        if (currentRate > _averageRate * _burstThreshold) {
            return MessagePriority::ELEVATED;  // Market move in progress
        }
        
        return MessagePriority::NORMAL;
    }
    
private:
    double _burstThreshold;
    double _averageRate = 0.0;
};

/**
 * @brief Time-based classifier - certain time windows are critical
 * (e.g., market open/close)
 */
class TimeWindowClassifier : public MessagePriorityClassifier {
public:
    // Add critical time window (start_ms, end_ms, priority)
    void addCriticalWindow(double startMs, double endMs, MessagePriority priority) {
        _windows.push_back({startMs, endMs, priority});
    }
    
    MessagePriority classify(size_t idx, const char* msg, const PlaybackState& state) override {
        double elapsed = state.elapsedMilliseconds();
        
        for (const auto& window : _windows) {
            if (elapsed >= window.startMs && elapsed <= window.endMs) {
                return window.priority;
            }
        }
        
        return MessagePriority::NORMAL;
    }
    
private:
    struct TimeWindow {
        double startMs;
        double endMs;
        MessagePriority priority;
    };
    std::vector<TimeWindow> _windows;
};

/**
 * @brief Priority-aware rate limit rule
 * Only enforces limits on NORMAL priority messages
 */
class PriorityAwareRateLimitRule : public PlaybackRule {
public:
    PriorityAwareRateLimitRule(size_t maxRate, 
                               std::shared_ptr<MessagePriorityClassifier> classifier)
        : _maxRate(maxRate), _classifier(classifier) {}
    
    Priority getPriority() const override { return Priority::SAFETY; }
    
    Decision apply(size_t idx, const char* msg,
                  const PlaybackState& state, Decision current) override {
        // Classify message
        MessagePriority msgPriority = _classifier->classify(idx, msg, state);
        
        // CRITICAL and EMERGENCY messages bypass rate limits
        if (msgPriority >= MessagePriority::CRITICAL) {
            // Allow through regardless of rate
            return current;
        }
        
        // NORMAL and ELEVATED messages subject to rate limit
        size_t currentRate = state.getCurrentRate();
        if (currentRate >= _maxRate) {
            // Exceeded rate limit
            if (msgPriority == MessagePriority::ELEVATED) {
                // Elevated messages get smaller backoff
                auto backoff = std::chrono::microseconds(500'000 / _maxRate);
                current.accumulatedDelay += backoff;
                current.outcome = Outcome::MODIFIED;
            } else {
                // Normal messages get full backoff (or VETO)
                current.outcome = Outcome::VETO;
            }
        }
        
        return current;
    }
    
private:
    size_t _maxRate;
    std::shared_ptr<MessagePriorityClassifier> _classifier;
};

/**
 * @brief Priority-aware burst rule
 * CRITICAL messages can break out of burst windows
 */
class PriorityAwareBurstRule : public PlaybackRule {
public:
    PriorityAwareBurstRule(size_t burstSize,
                          std::chrono::milliseconds burstInterval,
                          std::shared_ptr<MessagePriorityClassifier> classifier)
        : _burstSize(burstSize), _burstInterval(burstInterval), _classifier(classifier) {}
    
    Priority getPriority() const override { return Priority::CONTROL; }
    
    void onPlaybackStart() override {
        _lastBurstTime = std::chrono::steady_clock::now();
        _messagesInCurrentBurst = 0;
    }
    
    Decision apply(size_t idx, const char* msg,
                  const PlaybackState& state, Decision current) override {
        // Classify message
        MessagePriority msgPriority = _classifier->classify(idx, msg, state);
        
        // CRITICAL+ messages always go through immediately
        if (msgPriority >= MessagePriority::CRITICAL) {
            current.outcome = Outcome::SEND_NOW;
            current.accumulatedDelay = std::chrono::microseconds(0);
            return current;
        }
        
        // Normal burst logic for NORMAL and ELEVATED messages
        if (_messagesInCurrentBurst < _burstSize) {
            _messagesInCurrentBurst++;
            current.outcome = Outcome::SEND_NOW;
            current.accumulatedDelay = std::chrono::microseconds(0);
            return current;
        }
        
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
        
        // ELEVATED messages get shorter wait times
        auto waitTime = _burstInterval - elapsed;
        if (msgPriority == MessagePriority::ELEVATED) {
            waitTime = waitTime / 2;  // Half the wait for elevated messages
        }
        
        current.accumulatedDelay += std::chrono::duration_cast<std::chrono::microseconds>(waitTime);
        current.outcome = Outcome::MODIFIED;
        return current;
    }
    
private:
    size_t _burstSize;
    std::chrono::milliseconds _burstInterval;
    size_t _messagesInCurrentBurst = 0;
    std::chrono::steady_clock::time_point _lastBurstTime;
    std::shared_ptr<MessagePriorityClassifier> _classifier;
};
