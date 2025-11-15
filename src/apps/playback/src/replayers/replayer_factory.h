#pragma once

#include <memory>
#include <string>
#include <IPlaybackMarketData.h>
#include <replayers/types/udp_replayer.h>
#include <replayers/types/tcp_replayer.h>
#include <replayers/types/null_replayer.h>
#include <replayers/types/console_replayer.h>

namespace playback::replayer {

  /// @brief A factory method returning the method of transporting market data.
  /// @param senderType How/where to send data. Must be udp, tcp, console, or null. 
  /// @param address udp: the address (e.g., 127.0.0.1) used to broadcast the message.
  ///                tcp: the address of the remote machine to initiate a connection.
  /// @param port udp/tcp: the destination port number to send a message.
  /// @param ttl udp: the allowed number of hops across the network before being dropped.
  /// @return An abstraction you can simply start sending data to.
  std::unique_ptr<IPlaybackMarketData> createSender(
    const std::string& senderType,
    const std::string& address, // 
    uint16_t port, 
    uint8_t ttl) {
      if (senderType == "udp") {
        return std::make_unique<UdpMulticastMessageSender>(address, port, ttl);
      } 
      if (senderType == "tcp") {
        return std::make_unique<TcpMessageSender>(address, port);
      } 
      if (senderType == "console") {
        return std::make_unique<ConsoleMessageSender>();
      }  
      if (senderType == "null") {
        return std::make_unique<NullMessageSender>();
      }
      throw std::runtime_error("Unknown sender senderType: " + senderType);
  }
} // namespace playback::replayer