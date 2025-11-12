#pragma once

#include "message_sender.h"
#include "sender_types/udp_message_sender.h"
#include "sender_types/tcp_message_sender.h"
#include "sender_types/null_message_sender.h"
#include "sender_types/console_message_sender.h"
#include <memory>
#include <string>

namespace market_data_playback {

std::unique_ptr<IMessageSender> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl = 1);

}
