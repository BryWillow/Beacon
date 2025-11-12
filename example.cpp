#pragma once
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message_sender_interface.h"
#include <format>

namespace market_data_playback {
  class TcpMessageSender : public IPlaybackMarketData {
  public:
    explicit TcpMessageSender(uint16_t port) : _port(port), _messagesSent(0) {
      _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
      if (_listenSocket < 0) {
        throw std::runtime_error("Failed to create TCP socket: " + std::string(strerror(errno)));
      }
      int reuse = 1;
      if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "[TCP WARNING] Failed to set SO_REUSEADDR: " << strerror(errno) << "\n";
      }
      sockaddr_in serverAddr;
      std::memset(&serverAddr, 0, sizeof(serverAddr));
      serverAddr.sin_family = AF_INET;
      serverAddr.sin_addr.s_addr = INADDR_ANY;
      serverAddr.sin_port = htons(_port);
      if (bind(_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        close(_listenSocket);
        throw std::runtime_error("Failed to bind TCP socket to port " + std::to_string(_port) + 
                               ": " + std::string(strerror(errno)));
      }
      if (listen(_listenSocket, 1) < 0) {
        close(_listenSocket);
        throw std::runtime_error("Failed to listen on TCP socket: " + std::string(strerror(errno)));
      }
      std::cout << "[TCP] Listening on port " << _port << ", waiting for client connection...\n";
      sockaddr_in clientAddr;
      socklen_t clientAddrLen = sizeof(clientAddr);
      _clientSocket = accept(_listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
      if (_clientSocket < 0) {
        close(_listenSocket);
        throw std::runtime_error("Failed to accept client connection: " + std::string(strerror(errno)));
      }
      char clientIp[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
      std::cout << "[TCP] Client connected from " << clientIp << ":" << ntohs(clientAddr.sin_port) << "\n";
      int noDelay = 1;
      if (setsockopt(_clientSocket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay)) < 0) {
        std::cerr << "[TCP WARNING] Failed to set TCP_NODELAY: " << strerror(errno) << "\n";
      }
      int sendBufferSize = 2 * 1024 * 1024;
      if (setsockopt(_clientSocket, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize)) < 0) {
        std::cerr << "[TCP WARNING] Failed to set send buffer size: " << strerror(errno) << "\n";
      }
    }

    ~TcpMessageSender() override {
      if (_clientSocket >= 0) {
        close(_clientSocket);
      }
      if (_listenSocket >= 0) {
        close(_listenSocket);
      }
    }

    TcpMessageSender(const TcpMessageSender&) = delete;
    TcpMessageSender& operator=(const TcpMessageSender&) = delete;

    bool send(const char* message, size_t length) override {
      ssize_t sent = ::send(_clientSocket, message, length, 0);
      if (sent < 0) {
        std::cerr << "[TCP ERROR] Failed to send message: " << strerror(errno) << "\n";
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
      // Optional: Implement if needed
    }

    size_t getMessagesSent() const override { return _messagesSent; }

    uint16_t getPort() const { return _port; }

  private:
    uint16_t _port;
    int _listenSocket = -1;
    int _clientSocket = -1;
    size_t _messagesSent;
  };
}