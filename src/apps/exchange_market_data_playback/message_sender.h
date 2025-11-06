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
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>

/**
 * @brief Abstract interface for sending messages during playback
 */
class MessageSender {
public:
    virtual ~MessageSender() = default;
    
    // Send a message (returns true on success)
    virtual bool send(const char* message, size_t length) = 0;
    
    // Flush any buffered messages
    virtual void flush() {}
    
    // Get statistics about messages sent
    virtual size_t getMessagesSent() const = 0;
};

/**
 * @brief Console sender - prints messages to stdout (for debugging)
 */
class ConsoleSender : public MessageSender {
public:
    bool send(const char* message, size_t length) override {
        std::cout << "[SEND] Message " << _messagesSent << " (" << length << " bytes)\n";
        _messagesSent++;
        return true;
    }
    
    size_t getMessagesSent() const override { return _messagesSent; }
    
private:
    size_t _messagesSent = 0;
};

/**
 * @brief Null sender - discards messages (for dry-run testing)
 */
class NullSender : public MessageSender {
public:
    bool send(const char* message, size_t length) override {
        _messagesSent++;
        return true;
    }
    
    size_t getMessagesSent() const override { return _messagesSent; }
    
private:
    size_t _messagesSent = 0;
};

/**
 * @brief UDP Multicast sender - sends messages to a multicast group
 * 
 * Example usage:
 *   UdpMulticastSender sender("239.255.0.1", 12345);
 *   sender.send(message, 32);
 */
class UdpMulticastSender : public MessageSender {
public:
    UdpMulticastSender(const std::string& multicastAddress, uint16_t port, uint8_t ttl = 1)
        : _multicastAddress(multicastAddress), _port(port), _ttl(ttl), _messagesSent(0) {
        
        // Create UDP socket
        _socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (_socket < 0) {
            throw std::runtime_error("Failed to create UDP socket: " + std::string(strerror(errno)));
        }
        
        // Increase socket send buffer size (for high-frequency data)
        int sendBufferSize = 2 * 1024 * 1024; // 2 MB
        if (setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize)) < 0) {
            std::cerr << "[UDP WARNING] Failed to set send buffer size: " << strerror(errno) << "\n";
        }
        
        // Set TTL for multicast
        if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, &_ttl, sizeof(_ttl)) < 0) {
            close(_socket);
            throw std::runtime_error("Failed to set multicast TTL: " + std::string(strerror(errno)));
        }
        
        // Enable multicast loopback (CRITICAL for localhost testing)
        unsigned char loopback = 1;
        if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback, sizeof(loopback)) < 0) {
            std::cerr << "[UDP WARNING] Failed to enable multicast loopback: " << strerror(errno) << "\n";
        } else {
            std::cout << "[UDP] Multicast loopback enabled (localhost testing)\n";
        }
        
        // Use default interface (INADDR_ANY) - macOS routes multicast via en0
        in_addr localInterface;
        localInterface.s_addr = INADDR_ANY;
        if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, &localInterface, sizeof(localInterface)) < 0) {
            std::cerr << "[UDP WARNING] Failed to set multicast interface: " << strerror(errno) << "\n";
        }
        
        // Setup destination address
        std::memset(&_destAddr, 0, sizeof(_destAddr));
        _destAddr.sin_family = AF_INET;
        _destAddr.sin_port = htons(_port);
        if (inet_pton(AF_INET, _multicastAddress.c_str(), &_destAddr.sin_addr) <= 0) {
            close(_socket);
            throw std::runtime_error("Invalid multicast address: " + _multicastAddress);
        }
        
        std::cout << "[UDP] Multicast sender initialized: " << _multicastAddress << ":" << _port 
                  << " (TTL=" << static_cast<int>(_ttl) << ")\n";
    }
    
    ~UdpMulticastSender() {
        if (_socket >= 0) {
            close(_socket);
        }
    }
    
    // Disable copy
    UdpMulticastSender(const UdpMulticastSender&) = delete;
    UdpMulticastSender& operator=(const UdpMulticastSender&) = delete;
    
    bool send(const char* message, size_t length) override {
        ssize_t sent = sendto(_socket, message, length, 0, 
                             reinterpret_cast<const sockaddr*>(&_destAddr), 
                             sizeof(_destAddr));
        
        if (sent < 0) {
            std::cerr << "[UDP ERROR] Failed to send message: " << strerror(errno) << "\n";
            return false;
        }
        
        if (static_cast<size_t>(sent) != length) {
            std::cerr << "[UDP WARNING] Partial send: " << sent << "/" << length << " bytes\n";
            return false;
        }
        
        _messagesSent++;
        return true;
    }
    
    size_t getMessagesSent() const override { return _messagesSent; }
    
    const std::string& getMulticastAddress() const { return _multicastAddress; }
    uint16_t getPort() const { return _port; }
    
private:
    std::string _multicastAddress;
    uint16_t _port;
    uint8_t _ttl;
    int _socket = -1;
    sockaddr_in _destAddr;
    size_t _messagesSent;
};

/**
 * @brief File sender - writes messages to a binary file
 */
class FileSender : public MessageSender {
public:
    explicit FileSender(const std::string& filename) : _filename(filename), _messagesSent(0) {
        _file = std::fopen(filename.c_str(), "wb");
        if (!_file) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }
        std::cout << "[FILE] Writing to: " << filename << "\n";
    }
    
    ~FileSender() {
        if (_file) {
            std::fclose(_file);
        }
    }
    
    // Disable copy
    FileSender(const FileSender&) = delete;
    FileSender& operator=(const FileSender&) = delete;
    
    bool send(const char* message, size_t length) override {
        size_t written = std::fwrite(message, 1, length, _file);
        if (written != length) {
            std::cerr << "[FILE ERROR] Failed to write message\n";
            return false;
        }
        _messagesSent++;
        return true;
    }
    
    void flush() override {
        if (_file) {
            std::fflush(_file);
        }
    }
    
    size_t getMessagesSent() const override { return _messagesSent; }
    
private:
    std::string _filename;
    std::FILE* _file = nullptr;
    size_t _messagesSent;
};

/**
 * @brief TCP sender - sends messages to connected TCP clients
 * 
 * Listens on a port, accepts one client connection, and streams messages.
 * Example usage:
 *   TcpSender sender(12345);
 *   sender.send(message, 32);
 */
class TcpSender : public MessageSender {
public:
    explicit TcpSender(uint16_t port) : _port(port), _messagesSent(0) {
        // Create TCP socket
        _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (_listenSocket < 0) {
            throw std::runtime_error("Failed to create TCP socket: " + std::string(strerror(errno)));
        }
        
        // Allow address reuse (for rapid restarts)
        int reuse = 1;
        if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::cerr << "[TCP WARNING] Failed to set SO_REUSEADDR: " << strerror(errno) << "\n";
        }
        
        // Bind to port
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
        
        // Listen for connections
        if (listen(_listenSocket, 1) < 0) {
            close(_listenSocket);
            throw std::runtime_error("Failed to listen on TCP socket: " + std::string(strerror(errno)));
        }
        
        std::cout << "[TCP] Listening on port " << _port << ", waiting for client connection...\n";
        
        // Accept one client (blocking)
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        _clientSocket = accept(_listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
        
        if (_clientSocket < 0) {
            close(_listenSocket);
            throw std::runtime_error("Failed to accept client connection: " + std::string(strerror(errno)));
        }
        
        // Get client address for logging
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
        std::cout << "[TCP] Client connected from " << clientIp << ":" << ntohs(clientAddr.sin_port) << "\n";
        
        // Disable Nagle's algorithm for low latency
        int noDelay = 1;
        if (setsockopt(_clientSocket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay)) < 0) {
            std::cerr << "[TCP WARNING] Failed to set TCP_NODELAY: " << strerror(errno) << "\n";
        }
        
        // Set send buffer size
        int sendBufferSize = 2 * 1024 * 1024; // 2 MB
        if (setsockopt(_clientSocket, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, sizeof(sendBufferSize)) < 0) {
            std::cerr << "[TCP WARNING] Failed to set send buffer size: " << strerror(errno) << "\n";
        }
    }
    
    ~TcpSender() {
        if (_clientSocket >= 0) {
            close(_clientSocket);
        }
        if (_listenSocket >= 0) {
            close(_listenSocket);
        }
    }
    
    // Disable copy
    TcpSender(const TcpSender&) = delete;
    TcpSender& operator=(const TcpSender&) = delete;
    
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
    
    size_t getMessagesSent() const override { return _messagesSent; }
    
    uint16_t getPort() const { return _port; }
    
private:
    uint16_t _port;
    int _listenSocket = -1;
    int _clientSocket = -1;
    size_t _messagesSent;
};

// TODO: Add TcpSender implementation
