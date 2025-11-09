// ---------------------------------------------------------------------------
// @file        tcp_client.h
// Project     : Beacon
// Component   : HFT Networking
// Description : TCP client for low-latency order entry and execution reports
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstring>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

namespace beacon::hft::networking
{
    /**
     * @class TcpClient
     * @brief Thin RAII wrapper for TCP client connections (zero-copy design).
     *
     * Design goals:
     * - Beautiful interface: RAII socket management, clean connect/send/recv
     * - Zero overhead: send() and recv() are inline thin wrappers
     * - Hot-path optimized: TCP_NODELAY enabled by default
     * 
     * Usage:
     *   TcpClient client("127.0.0.1", 8080);
     *   client.send(order, sizeof(order));
     *   ssize_t n = client.recv(buffer, sizeof(buffer));
     */
    class TcpClient
    {
    public:
        /**
         * @brief Connect to TCP server
         * @param host Server hostname or IP address
         * @param port Server port
         * @param enableNoDelay Disable Nagle's algorithm for low latency (default true)
         * @param sendBufferSize Socket send buffer size in bytes (0 = system default)
         * @param recvBufferSize Socket receive buffer size in bytes (0 = system default)
         * @throws std::runtime_error if connection fails
         */
        TcpClient(const std::string& host, 
                 uint16_t port,
                 bool enableNoDelay = true,
                 size_t sendBufferSize = 0,
                 size_t recvBufferSize = 0)
            : _host(host), _port(port)
        {
            // === UGLY INTERNALS START (but necessary for correct setup) ===
            
            _socket = socket(AF_INET, SOCK_STREAM, 0);
            if (_socket < 0) {
                throw std::runtime_error("Failed to create TCP socket: " + 
                                       std::string(strerror(errno)));
            }

            // Setup server address
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(_port);

            if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
                close(_socket);
                throw std::runtime_error("Invalid address: " + host);
            }

            // Connect to server
            if (connect(_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                close(_socket);
                throw std::runtime_error("Failed to connect to " + host + ":" + 
                                       std::to_string(port) + ": " + 
                                       std::string(strerror(errno)));
            }

            // Disable Nagle's algorithm for low latency (most HFT apps want this)
            if (enableNoDelay) {
                int noDelay = 1;
                if (setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay)) < 0) {
                    // Non-fatal, just warn
                    // std::cerr << "[TCP WARNING] Failed to set TCP_NODELAY\n";
                }
            }

            // Set send buffer size if requested
            if (sendBufferSize > 0) {
                int bufSize = static_cast<int>(sendBufferSize);
                setsockopt(_socket, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize));
            }

            // Set receive buffer size if requested
            if (recvBufferSize > 0) {
                int bufSize = static_cast<int>(recvBufferSize);
                setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));
            }

            // === UGLY INTERNALS END ===
        }

        ~TcpClient()
        {
            if (_socket >= 0) {
                close(_socket);
            }
        }

        // Non-copyable, non-movable (socket is OS resource)
        TcpClient(const TcpClient&) = delete;
        TcpClient& operator=(const TcpClient&) = delete;
        TcpClient(TcpClient&&) = delete;
        TcpClient& operator=(TcpClient&&) = delete;

        /**
         * @brief Send data (blocking, hot-path optimized)
         * @param data Pointer to data to send
         * @param len Number of bytes to send
         * @return Number of bytes sent, or -1 on error
         * 
         * HOT PATH: Inline wrapper around system send(), zero overhead.
         * For guaranteed complete sends, use sendAll() instead.
         */
        [[nodiscard]] inline ssize_t send(const void* data, size_t len) noexcept
        {
            // Direct syscall, no fluff
            return ::send(_socket, data, len, 0);
        }

        /**
         * @brief Send all data (blocks until complete or error)
         * @param data Pointer to data to send
         * @param len Number of bytes to send
         * @return true if all bytes sent, false on error
         * 
         * Use this when you need guaranteed complete transmission.
         */
        [[nodiscard]] bool sendAll(const void* data, size_t len) noexcept
        {
            const char* ptr = static_cast<const char*>(data);
            size_t remaining = len;

            while (remaining > 0) {
                ssize_t sent = ::send(_socket, ptr, remaining, 0);
                if (sent <= 0) {
                    return false; // Error or connection closed
                }
                ptr += sent;
                remaining -= sent;
            }

            return true;
        }

        /**
         * @brief Receive data (blocking, hot-path optimized)
         * @param buffer User buffer to receive into
         * @param maxLen Maximum bytes to receive
         * @return Number of bytes received, 0 if connection closed, -1 on error
         * 
         * HOT PATH: Inline wrapper around system recv(), zero overhead.
         */
        [[nodiscard]] inline ssize_t recv(void* buffer, size_t maxLen) noexcept
        {
            // Direct syscall, no fluff
            return ::recv(_socket, buffer, maxLen, 0);
        }

        /**
         * @brief Get the underlying socket file descriptor (for advanced use)
         * @return Socket FD
         */
        [[nodiscard]] int fd() const noexcept { return _socket; }

        /**
         * @brief Get server host
         */
        [[nodiscard]] const std::string& host() const noexcept { return _host; }

        /**
         * @brief Get server port
         */
        [[nodiscard]] uint16_t port() const noexcept { return _port; }

        /**
         * @brief Check if socket is valid (connected)
         */
        [[nodiscard]] bool isConnected() const noexcept { return _socket >= 0; }

    private:
        int _socket = -1;
        std::string _host;
        uint16_t _port;
    };

} // namespace beacon::hft::networking
