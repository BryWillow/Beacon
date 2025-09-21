// ---------------------------------------------------------------------------
// File        : itch_message_udp_replayer.h
// Project     : Beacon
// Component   : Common
// Description : Replays ITCH message from a file over UDP
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

#include "itch_message.h"
#include "pinned_thread.h"
#include "cpu_pause.h"
#include "constants.h"  // defines NO_PINNING

/**
 * @class ItchMessageUdpReplayer
 * @brief Plays back ITCH messages over UDP at configurable speed.
 *
 * Features:
 * - Low-latency pinned thread replay
 * - Atomic stop flag for safe shutdown
 * - Configurable replay speed
 * - Memory-resident message loading and validation
 */
class ItchMessageUdpReplayer {
public:
    /**
     * @brief Construct a replayer
     * @param fileName Path to the ITCH message file
     * @param destIp Destination IP for UDP
     * @param destPort Destination UDP port
     * @param speedFactor Replay speed multiplier (1.0 = real-time)
     * @param cpuCore Optional CPU core to pin the thread (NO_PINNING = no pin)
     */
    ItchMessageUdpReplayer(const std::string& fileName,
                           const std::string& destIp,
                           uint16_t destPort,
                           double   speedFactor,
                           int      cpuCore = hft::NO_CPU_PINNING)
        : _fileName(fileName)
        , _destIp(destIp)
        , _destPort(destPort)
        , _speedFactor(speedFactor)
        , _cpuCore(cpuCore)
        , _stopFlag(false)
    {}

    /// Deleted copy and move constructors to enforce unique ownership
    ItchMessageUdpReplayer(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer(ItchMessageUdpReplayer&&) = delete;
    ItchMessageUdpReplayer& operator=(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer& operator=(ItchMessageUdpReplayer&&) = delete;

    /// @brief Start the replay thread
    void start() {
        _thread = std::make_unique<PinnedThread>(
            [this](std::atomic<bool>& stop) {
                replayLoop(stop);
            },
            _cpuCore,
            _stopFlag
        );
    }

    /// @brief Stop the replay thread
    void stop() {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread) _thread->join();
    }

    /// @brief Returns true if all messages have been replayed
    bool finished() const {
        return _current_index >= _total_messages;
    }

    /**
     * @brief Load all messages from file into memory and validate
     * @throws std::runtime_error if file cannot be opened or message invalid
     */
    void loadAllMessages() {
        std::ifstream ifs(_fileName, std::ios::binary);
        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open ITCH file: " + _fileName);
        }

        _messages.clear();
        ItchMessage msg;
        size_t index = 0;
        while (msg.deserialize(ifs)) {
            // Basic validation
            if (msg.symbol[0] == '\0' || msg.size == 0 || msg.price <= 0.0) {
                throw std::runtime_error("Invalid message at index " + std::to_string(index));
            }
            _messages.push_back(msg);
            ++index;
        }

        if (_messages.empty()) {
            throw std::runtime_error("No messages loaded from file: " + _fileName);
        }

        _total_messages = _messages.size();
    }

private:
    /// @brief Actual replay loop executed on the pinned thread
    void replayLoop(std::atomic<bool>& stop) {
        while (!stop.load(std::memory_order_relaxed) && _current_index < _total_messages) {
            const ItchMessage& msg = _messages[_current_index];
            // send message via UDP (implementation omitted)
            ++_current_index;
            _mm_pause(); // reduce CPU pressure while spinning
        }
    }

private:
    std::string             _fileName;
    std::string             _destIp;
    uint16_t                _destPort;
    double                  _speedFactor;
    int                     _cpuCore;
    std::atomic<bool>       _stopFlag;
    std::unique_ptr<PinnedThread> _thread;

    std::vector<ItchMessage> _messages;      ///< Memory-resident messages
    std::atomic<size_t>      _current_index{0};
    size_t                   _total_messages{0};
};
