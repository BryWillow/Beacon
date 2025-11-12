/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Abstract interface for message senders (UDP multicast, TCP,
 *               file output, etc.) used during market data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <cstddef>

namespace market_data_playback::message_senders {

class IMessageSender {
public:
  virtual ~IMessageSender() = default;
  virtual bool send(const char* message, size_t length) = 0;
  virtual void flush() {}
  virtual size_t getMessagesSent() const = 0;
};

} // namespace market_data_playback::message_senders

namespace market_data_playback::playback_advisors {

class PlaybackState; // Forward declaration

enum class MessagePriority {
  NORMAL = 0,
  ELEVATED = 1,
  CRITICAL = 2,
  EMERGENCY = 3
};

class MessagePriorityClassifier {
public:
  virtual ~MessagePriorityClassifier() = default;
  virtual MessagePriority classify(size_t messageIndex,
                                  const char* message,
                                  const PlaybackState& state) = 0;
};

} // namespace market_data_playback::playback_advisors