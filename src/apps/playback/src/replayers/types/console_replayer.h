#pragma once

#include <iostream>
#include <IPlaybackMarketData.h>

namespace playback::replayer {
  class ConsoleMessageSender : public IPlaybackMarketData {
  public:
    bool send(const char* message, size_t length) override {
      std::cout << "[SEND] Message " << _messagesSent << " (" << length << " bytes)\n";
      _messagesSent++;
      return true;
    }
    void flush() override {
      std::cout << std::flush;
    }
    size_t getMessagesSent() const override { return _messagesSent; }
  private:
    size_t _messagesSent = 0;
  };
}