/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Factory function for creating message senders used in market
 *               data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <memory>
#include <string>
#include "../../interfaces/IPlaybackMarketData.h"    // CMake
#include "../replayer_types/tcp_replayer.h"     // CMake
#include "../replayer_types/udp_replayer.h"     // CMake
#include "../replayer_types/console_replayer.h" // CMake
#include "../replayer_types/null_replayer.h"    // CMake

namespace playback::replayer {

std::unique_ptr<IPlaybackMarketData> createSender(const std::string& type,
                                             const std::string& address,
                                             uint16_t port,
                                             uint8_t ttl);

std::unique_ptr<IPlaybackMarketData> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl) {
  if (type == "udp") {
    return std::make_unique<UdpMulticastMessageSender>(address, port, ttl);
  } 
  else if (type == "tcp") {
    return std::make_unique<TcpMessageSender>(address, port);
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