#pragma once

#include "playback_types/playback_type_udp.h"
#include "playback_types/playback_type_file.h"
#include "playback_types/playback_type_tcp.h"
#include "playback_types/playback_type_console.h"
#include "playback_types/playback_type_null.h"
#include <memory>
#include <string>

inline std::unique_ptr<IPlaybackMarketData> createSender(const std::string& type, const std::string& address, uint16_t port, const std::string& filename, uint8_t ttl = 1) {
    if (type == "udp_multicast") {
        return std::make_unique<UdpMulticastSender>(address, port, ttl);
    } else if (type == "file") {
        return std::make_unique<FileWriter>(filename);
    } else if (type == "tcp") {
        return std::make_unique<TcpSender>(port);
    } else if (type == "console") {
        return std::make_unique<ConsoleMessageSender>();
    } else if (type == "null") {
        return std::make_unique<NullPlayback>();
    }
    throw std::runtime_error("Unknown sender type: " + type);
}
