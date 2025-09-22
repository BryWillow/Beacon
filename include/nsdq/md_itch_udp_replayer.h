// ---------------------------------------------------------------------------
// File        : itch_message_udp_replayer.h
// Project     : HftSimulator
// Component   : Common
// Description : Replays ITCH message from a file over UDP
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <memory>
#include <string>

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
        return _current_index >= total_messages_;
    }

private:
    /// @brief Actual replay loop executed on the pinned thread
    void replayLoop(std::atomic<bool>& stop) {
        while (!stop.load(std::memory_order_relaxed)) {
            ItchMessage msg{}; // populate message from file/memory
            // send message via UDP
            _mm_pause();       // reduce CPU pressure while spinning
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
    std::atomic<size_t>     _current_index{0};
    size_t                  total_messages_{0};
};
