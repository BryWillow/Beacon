#pragma once

#include <memory>
#include <string>

#include "../message_senders/message_sender_types/udp_message_sender.h"
#include "../message_senders/message_sender_types/tcp_message_sender.h"
#include "../message_senders/message_sender_types/console_message_sender.h"
#include "../message_senders/message_sender_types/null_message_sender.h"

namespace market_data_playback {
inline std::unique_ptr<IPlaybackMarketData> createSender(const std::string& type, const std::string& address, uint16_t port, const std::string& filename, uint8_t ttl = 1) {
    if (type == "udp_multicast"){
        return std::make_unique<UdpMulticastMessageSender>(address, port, ttl);
    } 
    else if (type == "tcp"){
        return std::make_unique<TcpMessageSender>(port);
    } 
    else if (type == "console") {
        return std::make_unique<ConsoleMessageSender>();
    } 
    else if (type == "null") {
        return std::make_unique<NullMessageSender>();
    }
    throw std::runtime_error("Unknown sender type: " + type);
}
}