#pragma once

#include <iostream>
#include <string>
#include <hft/networking/udp_socket.h>
#include "../../interfaces/IPlaybackMarketData.h" // CMake

namespace playback::replayer {
  class UdpMulticastMessageSender : public IPlaybackMarketData {

    private:
      beacon::hft::networking::UdpSocket _udp;
      size_t _messagesSent;

    public:
      UdpMulticastMessageSender(
        const std::string& multicastAddress, uint16_t port, uint8_t ttl = 1)
        : _udp(multicastAddress, port, ttl), _messagesSent(0) {}

    UdpMulticastMessageSender(const UdpMulticastMessageSender&) = delete;
    UdpMulticastMessageSender& operator=(const UdpMulticastMessageSender&) = delete;

    bool send(const char* message, size_t length) override {
      ssize_t sent = _udp.send(message, length);

      if (sent < 0) {
        std::cerr << "[UDP ERROR] Failed to send message\n";
        return false;
      }

      if (static_cast<size_t>(sent) != length) {
        std::cerr << "[UDP WARNING] Partial send: " << sent << "/" << length << " bytes\n";
        return false;
      }

      _messagesSent++;
      return true;
    }

    void flush() override {
      // No-op for UDP
    }

    size_t getMessagesSent() const override { return _messagesSent; }

    int fd() const { return _udp.fd(); }
    std::string address() const { return _udp.address(); }
    uint16_t port() const { return _udp.port(); }
  };
}