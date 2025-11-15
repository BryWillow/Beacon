#pragma once

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <string>
#include <hft/networking/tcp_client.h>
#include <IPlaybackMarketData.h>

namespace playback::replayer {
  class TcpMessageSender : public IPlaybackMarketData {
  public:
    TcpMessageSender(const std::string& host, uint16_t port)
      : _client(host, port), _messagesSent(0) {}

    // Disable copy
    TcpMessageSender(const TcpMessageSender&) = delete;
    TcpMessageSender& operator=(const TcpMessageSender&) = delete;

    bool send(const char* message, size_t length) override {
      ssize_t sent = _client.send(message, length);
      if (sent < 0) {
        std::cerr << "[TCP ERROR] Failed to send message\n";
        return false;
      }
      if (static_cast<size_t>(sent) != length) {
        std::cerr << "[TCP WARNING] Partial send: " << sent << "/" << length << " bytes\n";
        return false;
      }
      _messagesSent++;
      return true;
    }

    void flush() override {
      // No-op for TCP
    }

    size_t getMessagesSent() const override { return _messagesSent; }

    bool isConnected() const { return _client.isConnected(); }
    int fd() const { return _client.fd(); }
    std::string host() const { return _client.host(); }
    uint16_t port() const { return _client.port(); }

  private:
    beacon::hft::networking::TcpClient _client;
    size_t _messagesSent;
  };
}