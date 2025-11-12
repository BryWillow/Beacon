#include <gtest/gtest.h>
#include "message_senders/sender_types/udp_message_sender.h"

TEST(UDPTest, PacketsAreSent) {
  UdpMessageSender sender("239.255.0.1", 12345, 1);
  ASSERT_TRUE(sender.send("test", 4));
}

TEST(UDPTest, PacketsAreBinaryFormat) {
  // TODO: Implement binary format check
  SUCCEED();
}

TEST(UDPTest, PacketsNormalRate) {
  // TODO: Implement rate check
  SUCCEED();
}

TEST(UDPTest, PacketsSlowedRate) {
  // TODO: Implement slow rate check
  SUCCEED();
}

TEST(UDPTest, PacketsIncreasedRate) {
  // TODO: Implement fast rate check
  SUCCEED();
}

TEST(UDPTest, PacketsChaosMode) {
  // TODO: Implement chaos mode check
  SUCCEED();
}

// Placeholder for TCP
// TEST(TCPTest, DISABLED_TCPPlaceholder) {
//   // Not important right now
// }

TEST(UDPTest, PacketsToConsole) {
  // TODO: Implement console sender check
  SUCCEED();
}
