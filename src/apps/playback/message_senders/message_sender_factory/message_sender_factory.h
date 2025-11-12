#pragma once

#include <memory>
#include <string>

#include "../message_sender.h"
#include "../message_sender_types/udp_message_sender.h"
#include "../message_sender_types/tcp_message_sender.h"
#include "../message_sender_types/null_message_sender.h"
#include "../message_sender_types/console_message_sender.h"

namespace market_data_playback {

  std::unique_ptr<IMessageSender> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl = 1);

}
