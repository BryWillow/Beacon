#pragma once

#include <memory>
#include <string>

#include "../../interfaces/IPlaybackMarketData.h"    // CMake 
#include "../replayer_types/udp_replayer.h"
#include "../replayer_types/tcp_replayer.h"
#include "../replayer_types/null_replayer.h"
#include "../replayer_types/console_replayer.h"

namespace playback::replayer {
  std::unique_ptr<IPlaybackMarketData> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl = 1);
}
