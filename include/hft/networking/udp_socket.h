#pragma once

#include <string>
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace beacon::hft::networking {

  class UdpSocket {
  public:
    UdpSocket(const std::string& address, uint16_t port, uint8_t ttl = 1)
      : _address(address), _port(port) {
      _socket = socket(AF_INET, SOCK_DGRAM, 0);
      if (_socket < 0) {
        throw std::runtime_error("Failed to create UDP socket: " + std::string(strerror(errno)));
      }

      // Set TTL for multicast
      setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

      // Enable multicast loopback
      unsigned char loopback = 1;
      setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback, sizeof(loopback));

      // Use default interface (INADDR_ANY)
      in_addr localInterface;
      localInterface.s_addr = INADDR_ANY;
      setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface));

      std::memset(&_destAddr, 0, sizeof(_destAddr));
      _destAddr.sin_family = AF_INET;
      _destAddr.sin_port = htons(_port);
      if (inet_pton(AF_INET, _address.c_str(), &_destAddr.sin_addr) <= 0) {
        close(_socket);
        throw std::runtime_error("Invalid UDP address: " + _address);
      }
    }

    ~UdpSocket() {
      if (_socket >= 0) close(_socket);
    }

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    ssize_t send(const void* data, size_t len) {
      return sendto(_socket, data, len, 0, reinterpret_cast<const sockaddr*>(&_destAddr), sizeof(_destAddr));
    }

    ssize_t recv(void* buffer, size_t len, sockaddr_in* srcAddr = nullptr) {
      socklen_t addrLen = sizeof(sockaddr_in);
      sockaddr_in tmpAddr;
      sockaddr_in* addrPtr = srcAddr ? srcAddr : &tmpAddr;
      return recvfrom(_socket, buffer, len, 0, reinterpret_cast<sockaddr*>(addrPtr), &addrLen);
    }

    int fd() const { return _socket; }
    std::string address() const { return _address; }
    uint16_t port() const { return _port; }

  private:
    int _socket = -1;
    std::string _address;
    uint16_t _port;
    sockaddr_in _destAddr;
  };

}
