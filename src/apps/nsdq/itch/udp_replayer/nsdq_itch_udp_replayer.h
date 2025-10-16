#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdlib>

// Absolute includes
#include "hft/ringbuffer/spsc_ringbuffer.h"
#include "nsdq/market_data/itch/current/messages_active.h"
#include "hft/networking/hton_utils.h"
namespace nsdq = beacon::nsdq::market_data::itch;
using ITCHMessage = nsdq::ITCHMessage;

class ItchMessageUdpPlayer {
public:
    ItchMessageUdpPlayer(const std::string& filename, uint16_t port)
        : _filename(filename), _port(port), _stop_flag(false) {}

    void run() {
        load_messages();
        replay_loop();
    }

    void stop() { _stop_flag = true; }

private:
    std::string _filename;
    uint16_t _port;
    std::atomic<bool> _stop_flag;

    std::vector<ITCHMessage> _messages;

    void load_messages() {
        std::ifstream ifs(_filename, std::ios::binary);
        if (!ifs) throw std::runtime_error("Cannot open file: " + _filename);

        ITCHMessage msg;
        while (ifs.read(reinterpret_cast<char*>(&msg), sizeof(msg))) {
            _messages.push_back(msg);
        }

        std::cout << "Loaded " << _messages.size() << " messages from " << _filename << "\n";
    }

    void replay_loop() {
        for (size_t i = 0; !_stop_flag && i < _messages.size(); ++i) {
            ITCHMessage& msg = _messages[i];

            // Here we would send msg over UDP
            // For demo, just print first 5 messages
            if (i < 5) {
                std::cout << "Replaying message type=" << msg.type
                          << " price=" << static_cast<int>(msg.price)
                          << " size=" << static_cast<int>(msg.size) << "\n";
            }

            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    }
};
