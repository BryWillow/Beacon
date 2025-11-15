#pragma once

#include <IPlaybackMarketData.h>

namespace playback::replayer {
  class NullMessageSender : public IPlaybackMarketData {
  public:
    NullMessageSender() = default;
    ~NullMessageSender() override = default;

    NullMessageSender(const NullMessageSender&) = delete;
    NullMessageSender& operator=(const NullMessageSender&) = delete;

    bool send(const char* message, size_t length) override {
      (void)message;
      (void)length;
      _messagesSent++;
      return true;
    }

    void flush() override {}

    size_t getMessagesSent() const override { return _messagesSent; }

  private:
    size_t _messagesSent = 0;
  };
}