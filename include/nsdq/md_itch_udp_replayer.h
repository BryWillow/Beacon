// ---------------------------------------------------------------------------
// File        : md_itch_udp_replayer.h
// Project     : Beacon
// Component   : Common
// Description : Replays ITCH messages from a file over UDP with low-latency
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "md_itch_message.h"
#include "utils/pinned_thread.h"
#include "utils/cpu_pause.h"
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
    ItchMessageUdpReplayer(const std::string& fileName,
                           const std::string& destIp,
                           uint16_t destPort,
                           double speedFactor,
                           int cpuCore = hft::NO_CPU_PINNING)
        : _fileName(fileName)
        , _destIp(destIp)
        , _destPort(destPort)
        , _speedFactor(speedFactor)
        , _cpuCore(cpuCore)
        , _stopFlag(false)
    {}

    ItchMessageUdpReplayer(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer(ItchMessageUdpReplayer&&) = delete;
    ItchMessageUdpReplayer& operator=(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer& operator=(ItchMessageUdpReplayer&&) = delete;

    void start() {
        _thread = std::make_unique<PinnedThread>(
            [this](std::atomic<bool>& stop) { replayLoop(stop); },
            _cpuCore,
            _stopFlag
        );
    }

    void stop() {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread) _thread->join();
    }

    bool finished() const {
        return _currentIndex >= _totalMessages;
    }

    void loadAllMessages() {
        std::ifstream inFile(_fileName, std::ios::binary | std::ios::ate);
        if (!inFile) {
            throw std::runtime_error("Failed to open ITCH message file: " + _fileName);
        }

        auto fileSize = inFile.tellg();
        if (fileSize % sizeof(ItchMessage) != 0) {
            throw std::runtime_error("File size is not aligned with ItchMessage size");
        }

        _totalMessages = static_cast<size_t>(fileSize / sizeof(ItchMessage));
        _messages.reserve(_totalMessages);

        inFile.seekg(0, std::ios::beg);
        _messages.resize(_totalMessages);
        inFile.read(reinterpret_cast<char*>(_messages.data()), fileSize);
    }

private:
    void replayLoop(std::atomic<bool>& stop) {
        while (!stop.load(std::memory_order_relaxed) && _currentIndex < _totalMessages) {
            const auto& msg = _messages[_currentIndex++];
            // TODO: send msg over UDP
            _mm_pause(); // reduce CPU pressure while spinning
        }
    }

private:
    std::string _fileName;
    std::string _destIp;
    uint16_t _destPort;
    double _speedFactor;
    int _cpuCore;
    std::atomic<bool> _stopFlag;
    std::unique_ptr<PinnedThread> _thread;
    std::atomic<size_t> _currentIndex{0};
    size_t _totalMessages{0};

    std::vector<ItchMessage> _messages; ///< Preloaded ITCH messages in memory
};