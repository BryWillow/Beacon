/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Factory function for creating message senders used in market
 *               data playback.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include <memory>
#include <string>
#include <IMessageSender.h>

namespace market_data_playback {

std::unique_ptr<IMessageSender> createSender(const std::string& type,
                                             const std::string& address,
                                             uint16_t port,
                                             uint8_t ttl);

} // namespace market_data_playback

namespace market_data_playback {

std::unique_ptr<IMessageSender> createSender(const std::string& type, const std::string& address, uint16_t port, uint8_t ttl) {
  if (type == "udp") {
    return std::make_unique<UdpMulticastMessageSender>(address, port, ttl);
  } else if (type == "tcp") {
    return std::make_unique<TcpMessageSender>(address, port);
  } else if (type == "console") {
    return std::make_unique<ConsoleMessageSender>();
  } else if (type == "null") {
    return std::make_unique<NullMessageSender>();
  }
  throw std::runtime_error("Unknown sender type: " + type);
}

}