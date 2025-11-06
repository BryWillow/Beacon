// ---------------------------------------------------------------------------
// @file        udp_receiver.h
// Project     : Beacon
// Component   : HFT Networking
// Description : UDP multicast receiver for low-latency market data
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstring>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace beacon::hft::networking
{
    /**
     * @class UdpMulticastReceiver
     * @brief Thin RAII wrapper for UDP multicast reception (zero-copy design).
     *
     * Design goals:
     * - Beautiful interface: RAII socket management, clean API
     * - Zero overhead: recv() is inline thin wrapper around system call
     * - Hot-path optimized: Minimal instructions between kernel and user buffer
     * 
     * Usage:
     *   UdpMulticastReceiver receiver("239.255.0.1", 12345);
     *   char buffer[2048];
     *   ssize_t n = receiver.recv(buffer, sizeof(buffer));
     */
    class UdpMulticastReceiver
    {
    public:
        /**
         * @brief Construct and bind to multicast group
         * @param multicastAddr Multicast group address (e.g., "239.255.0.1")
         * @param port UDP port to listen on
         * @param recvBufferSize Socket receive buffer size in bytes (default 2MB)
         * @throws std::runtime_error if socket setup fails
         */
        UdpMulticastReceiver(const std::string& multicastAddr, 
                            uint16_t port, 
                            size_t recvBufferSize = 2 * 1024 * 1024)
            : _multicastAddr(multicastAddr), _port(port)
        {
            // === UGLY INTERNALS START (but necessary for correct setup) ===
            
            _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (_socket < 0) {
                throw std::runtime_error("Failed to create UDP socket: " + 
                                       std::string(strerror(errno)));
            }

            // Allow address reuse (for rapid restarts)
            int reuse = 1;
            if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
                close(_socket);
                throw std::runtime_error("Failed to set SO_REUSEADDR: " + 
                                       std::string(strerror(errno)));
            }
            
            // Allow port reuse (helpful for macOS multicast, but not critical)
            #ifdef SO_REUSEPORT
            if (setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
                // Non-fatal on some systems
                std::cerr << "[UDP WARNING] Failed to set SO_REUSEPORT (non-fatal): " << strerror(errno) << "\n";
            }
            #endif

            // Increase receive buffer for high-frequency data
            int bufSize = static_cast<int>(recvBufferSize);
            if (setsockopt(_socket, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize)) < 0) {
                // Non-fatal, just warn (some systems have limits)
                // std::cerr << "[UDP WARNING] Failed to set receive buffer size\n";
            }

            // Bind to the port (INADDR_ANY to receive multicast)
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(_port);

            if (bind(_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                close(_socket);
                throw std::runtime_error("Failed to bind to port " + std::to_string(_port) + 
                                       ": " + std::string(strerror(errno)));
            }

            // Join multicast group
            ip_mreq mreq;
            std::memset(&mreq, 0, sizeof(mreq));
            if (inet_pton(AF_INET, multicastAddr.c_str(), &mreq.imr_multiaddr) != 1) {
                close(_socket);
                throw std::runtime_error("Invalid multicast address: " + multicastAddr);
            }
            mreq.imr_interface.s_addr = INADDR_ANY;

            if (setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                close(_socket);
                throw std::runtime_error("Failed to join multicast group " + multicastAddr + 
                                       ": " + std::string(strerror(errno)));
            }

            // === UGLY INTERNALS END ===
        }

        ~UdpMulticastReceiver()
        {
            if (_socket >= 0) {
                close(_socket);
            }
        }

        // Non-copyable, non-movable (socket is OS resource)
        UdpMulticastReceiver(const UdpMulticastReceiver&) = delete;
        UdpMulticastReceiver& operator=(const UdpMulticastReceiver&) = delete;
        UdpMulticastReceiver(UdpMulticastReceiver&&) = delete;
        UdpMulticastReceiver& operator=(UdpMulticastReceiver&&) = delete;

        /**
         * @brief Receive a datagram (blocking, hot-path optimized)
         * @param buffer User buffer to receive into
         * @param maxLen Maximum bytes to receive
         * @return Number of bytes received, or -1 on error
         * 
         * HOT PATH: This function is inline and compiles to a single syscall.
         * Zero abstractions, zero overhead, zero branches.
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
         * @brief Get multicast address
         */
        [[nodiscard]] const std::string& multicastAddress() const noexcept { 
            return _multicastAddr; 
        }

        /**
         * @brief Get port
         */
        [[nodiscard]] uint16_t port() const noexcept { return _port; }

    private:
        int _socket = -1;
        std::string _multicastAddr;
        uint16_t _port;
    };

} // namespace beacon::hft::networking
