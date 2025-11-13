#pragma once

#include <memory>
#include <string>

#include "../message_sender.h"
#include <udp_replayer>
#include <tcp_replayer>
#include <null_replayer>
#include <console_replayer>

namespace market_data_playback {

  std::unique_ptr<IMessageSender> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl = 1);

}
