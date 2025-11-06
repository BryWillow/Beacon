// =============================================================================
// Project:      Beacon Tests
// File:         test_tcp_client.cpp
// Purpose:      GoogleTest suite for TCP client
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/networking/tcp_client.h"

using namespace hft::networking;

TEST(TCPClientTest, ConnectToInvalidAddress) {
    TCPClient client;
    EXPECT_FALSE(client.connect("999.999.999.999", 9999));
}

TEST(TCPClientTest, ConnectToUnreachablePort) {
    TCPClient client;
    // Port unlikely to be open
    EXPECT_FALSE(client.connect("127.0.0.1", 59999));
}

TEST(TCPClientTest, DisconnectWithoutConnection) {
    TCPClient client;
    // Should not crash
    client.disconnect();
}
