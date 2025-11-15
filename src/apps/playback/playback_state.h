/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Tracks the current state of playback including timing,
 *               message counts, and rates for use by playback rules.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <deque>

class PlaybackState {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration = std::chrono::steady_clock::duration;
    
    PlaybackState() : _startTime(std::chrono::steady_clock::now()) {}
    
    // Record that a message was sent
    void recordSent() {
        _messagesSent++;
        _totalMessagesSent++;
        auto now = std::chrono::steady_clock::now();
        _recentSendTimes.push_back(now);
        
        // Keep only last second of send times for rate calculation
        auto oneSecondAgo = now - std::chrono::seconds(1);
        while (!_recentSendTimes.empty() && _recentSendTimes.front() < oneSecondAgo) {
            _recentSendTimes.pop_front();
        }
    }
    
    // Record that a message was dropped
    void recordDropped() {
        _messagesDropped++;
    }
    
    // Record that a message was queued
    void recordQueued() {
        _messagesQueued++;
    }
    
    // Get elapsed time since playback started
    Duration elapsedTime() const {
        return std::chrono::steady_clock::now() - _startTime;
    }
    
    // Get elapsed time in milliseconds
    double elapsedMilliseconds() const {
        return std::chrono::duration<double, std::milli>(elapsedTime()).count();
    }
    
    // Get elapsed time in seconds
    double elapsedSeconds() const {
        return std::chrono::duration<double>(elapsedTime()).count();
    }
    
    // Get current message rate (messages per second based on last second)
    size_t getCurrentRate() const {
        return _recentSendTimes.size();
    }
    
    // Get total messages sent
    size_t getTotalSent() const { return _totalMessagesSent; }
    
    // Get messages sent since last reset
    size_t getMessagesSent() const { return _messagesSent; }
    
    // Get messages dropped
    size_t getMessagesDropped() const { return _messagesDropped; }
    
    // Get messages queued
    size_t getMessagesQueued() const { return _messagesQueued; }
    
    // Reset counters (for burst tracking, etc.)
    void reset() {
        _messagesSent = 0;
        _messagesDropped = 0;
        _messagesQueued = 0;
    }
    
    // Get start time
    TimePoint getStartTime() const { return _startTime; }
    
private:
    TimePoint _startTime;
    size_t _messagesSent = 0;
    size_t _totalMessagesSent = 0;
    size_t _messagesDropped = 0;
    size_t _messagesQueued = 0;
    std::deque<TimePoint> _recentSendTimes;  // For rate calculation
};
