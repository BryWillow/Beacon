// ---------------------------------------------------------------------------
// File        : itch_udp_listener.h
// Project     : HftSimulator
// Component   : Listener
// Description : High-performance UDP listener for ITCH messages
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <thread>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "md_itch_message.h"
#include "spsc_ringbuffer.h"
#include "utils/pinned_thread.h"
#include "utils/cpu_pause.h"
#include "constants.h"

/**
 * @class ItchUdpListener
 * @brief High-performance UDP listener for ITCH-style messages.
 *
 * Features:
 * - Non-blocking UDP socket for low-latency reception.
 * - Hot-path callback to process each message.
 * - Relaxed atomic stop flag.
 * - Optional CPU pinning for thread.
 *
 * Performance (~per message):
 * - recvfrom non-blocking: 50-100 cycles
 * - toHostOrder conversion: 10-20 cycles
 * - callback execution: 5-20 cycles
 * - atomic load _shouldStop: ~5 cycles
 */
template <typename Callback = std::function<void(const ItchMessage&)>>
class ItchUdpListener {
public:
    explicit ItchUdpListener(uint16_t udpPort, Callback callback, int cpuCore = hft::NO_CPU_PINNING)
        : _udpPort(udpPort),
          _cpuCore(cpuCore),
          _callback(std::move(callback)),
          _sDesc(-1),
          _shouldStop(false),
          _isRunning(false)
    {
        if (_cpuCore < hft::NO_CPU_PINNING)
            throw std::invalid_argument("Invalid CPU core index: " + std::to_string(_cpuCore));
    }

    ~ItchUdpListener() noexcept { stop(); }

    /// Deleted copy and move constructors to enforce unique ownership
    ItchUdpListener(const ItchUdpListener&) = delete;
    ItchUdpListener& operator=(const ItchUdpListener&) = delete;
    ItchUdpListener(ItchUdpListener&&) = delete;
    ItchUdpListener& operator=(ItchUdpListener&&) = delete;

    /// @brief Start listener thread
    /// @throws runtime_error if the listener is already running
    void start() {
        if (_isRunning.load(std::memory_order_relaxed))
            throw std::runtime_error("ItchUdpListener already running");
        _thread = std::thread([this]{ runHotPath(); });
        _isRunning.store(true, std::memory_order_relaxed);
    }

    /// @brief Stop listener and clean resources
    void stop() noexcept {
        _shouldStop.store(true, std::memory_order_relaxed);
        if (_thread.joinable()) _thread.join();
        if (_sDesc >= 0) close(_sDesc), _sDesc = -1;
        _isRunning.store(false, std::memory_order_relaxed);
    }

    /// @brief Returns true if running
    bool running() const noexcept { return _isRunning.load(std::memory_order_relaxed); }

    /// @brief Returns true if stop requested
    bool stopRequested() const noexcept { return _shouldStop.load(std::memory_order_relaxed); }

private:
    uint16_t            _udpPort;
    int                 _cpuCore;
    Callback            _callback;
    int                 _sDesc;
    std::atomic<bool>   _shouldStop;
    std::atomic<bool>   _isRunning;
    std::thread         _thread;

    /// @brief Create and bind UDP socket
    void setupSocket() {
        _sDesc = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sDesc < 0) throw std::runtime_error("Failed to create UDP socket");

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(_udpPort);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(_sDesc, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            int err = errno;
            close(_sDesc);
            _sDesc = -1;
            throw std::runtime_error(std::string("Failed to bind UDP socket: ") + std::strerror(err));
        }
    }

    /// @brief Pin thread to a specific CPU core
    void pinThreadToCore() {
        if (_cpuCore == hft::NO_CPU_PINNING) return;

#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(_cpuCore, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        (void)_cpuCore; // macOS does not support easy affinity
#endif
    }

    /// @brief Hot-path receiving loop
    inline void runHotPath() {
        setupSocket();
        pinThreadToCore();

        while (!_shouldStop.load(std::memory_order_relaxed)) {
            ItchMessage msg{};
            ssize_t bytesReceived = recvfrom(_sDesc, &msg, sizeof(msg), MSG_DONTWAIT, nullptr, nullptr);

            if (bytesReceived == sizeof(msg)) {
                msg.toHostOrder();
                _callback(msg);
            } else {
                _mm_pause(); // reduce CPU pressure while spinning
            }
        }

        _isRunning.store(false, std::memory_order_relaxed);
    }
};