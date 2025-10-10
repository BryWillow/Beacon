// ---------------------------------------------------------------------------
// @file        md_itch_udp_replayer.h
// Project     : Beacon
// Component   : NSDQ ITCH Utilities
// Description : Replays ITCH messages from a file over UDP with low-latency
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "constants.h"
#include "include/hft/concurrency/pinned_thread.h"
#include "include/hft/core/cpu_pause.h"
#include "itch_messages.h"  // updated ITCH structs

namespace bhcore = beacon::hft::core;
namespace bhthread = beacon::hft::concurrency;
namespace bhitch = beacon::nsdq::market_data::itch;

namespace beacon::nsdq::market_data::itch {

/**
 * @class ItchMessageUdpReplayer
 * @brief Reads ITCH messages from a file and replays them over UDP.
 *
 * Features:
 * - Fully in-memory playback for low-latency testing
 * - Optional CPU core pinning for deterministic performance
 * - Supports AddOrder, Trade, Cancel, Delete messages
 */
class ItchMessageUdpReplayer {
   public:
    /**
     * @param fileName Path to ITCH capture file
     * @param destIp   Destination IP for UDP
     * @param destPort Destination port
     * @param speedFactor Speed multiplier (not yet used in loop)
     * @param cpuCore CPU core to pin thread to (-1 disables pinning)
     */
    ItchMessageUdpReplayer(const std::string& fileName, const std::string& destIp,
                           uint16_t destPort, double speedFactor,
                           int cpuCore = hft::NO_CPU_PINNING)
        : _fileName(fileName),
          _destIp(destIp),
          _destPort(destPort),
          _speedFactor(speedFactor),
          _cpuCore(cpuCore),
          _stopFlag(false) {
        _sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock < 0)
            throw std::runtime_error("Failed to create UDP socket");

        std::memset(&_destAddr, 0, sizeof(_destAddr));
        _destAddr.sin_family = AF_INET;
        _destAddr.sin_port = htons(destPort);
        if (inet_pton(AF_INET, destIp.c_str(), &_destAddr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid destination IP: " + destIp);
        }
    }

    ~ItchMessageUdpReplayer() {
        if (_sock >= 0)
            close(_sock);
    }

    // Non-copyable, non-movable
    ItchMessageUdpReplayer(const ItchMessageUdpReplayer&) = delete;
    ItchMessageUdpReplayer& operator=(const ItchMessageUdpReplayer&) = delete;

    /** @brief Starts the replay thread */
    void start() {
        _thread = std::make_unique<bhthread::PinnedThread>(
            [this](std::atomic<bool>& stop) { replayLoop(stop); }, _cpuCore, _stopFlag);
    }

    /** @brief Stops the replay thread */
    void stop() {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread)
            _thread->stop();
    }

    /** @brief Check if all messages have been replayed */
    bool finished() const { return _currentIndex >= _rawData.size(); }

    /** @brief Loads all messages from file into memory for fast playback */
    void loadAllMessages() {
        std::ifstream inFile(_fileName, std::ios::binary | std::ios::ate);
        if (!inFile)
            throw std::runtime_error("Failed to open ITCH file: " + _fileName);

        auto fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        _rawData.resize(fileSize);
        inFile.read(reinterpret_cast<char*>(_rawData.data()), fileSize);

        _currentIndex = 0;
    }

   private:
    /** @brief Hot-path replay loop */
    void replayLoop(std::atomic<bool>& stop) {
        size_t offset = 0;

        while (!stop.load(std::memory_order_relaxed) && offset < _rawData.size()) {
            uint8_t msgTypeByte = _rawData[offset];
            auto msgType = static_cast<bhitch::MessageType>(msgTypeByte);
            size_t msgSize = messageSize(msgType);
            ::sendto(_sock, _rawData.data() + offset, msgSize, 0,
                     reinterpret_cast<struct sockaddr*>(&_destAddr), sizeof(_destAddr));

            offset += msgSize;

            // Reduce busy-wait CPU pressure
            bhcore::cpu_pause();
        }

        _currentIndex = offset;
    }

    /** @brief Returns the size of a message based on its MessageType */
    static size_t messageSize(bhitch::MessageType type) {
        using namespace bhitch;
        switch (type) {
            case MessageType::AddOrder:
                return sizeof(AddOrderMessage);
            case MessageType::Trade:
                return sizeof(TradeMessage);
            case MessageType::OrderCancel:
                return sizeof(OrderCancelMessage);
            case MessageType::OrderDelete:
                return sizeof(OrderDeleteMessage);
            case MessageType::OrderExecuted:
                return sizeof(OrderExecutedMessage);
            case MessageType::ReplaceOrder:
                return sizeof(ReplaceOrderMessage);
            case MessageType::MarketDepth:
                return sizeof(MarketDepthMessage);
            default:
                throw std::runtime_error("Unknown ITCH message type");
        }
    }

   private:
    std::string _fileName;
    std::string _destIp;
    uint16_t _destPort;
    double _speedFactor;
    int _cpuCore;

    std::atomic<bool> _stopFlag;
    std::unique_ptr<bhthread::PinnedThread> _thread;

    int _sock{-1};
    struct sockaddr_in _destAddr;

    std::vector<uint8_t> _rawData;
    size_t _currentIndex{0};
};

}  // namespace beacon::nsdq::market_data::itch
