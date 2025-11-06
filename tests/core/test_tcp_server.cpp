// =============================================================================
// Project:      Beacon Tests
// File:         test_tcp_server.cpp
// Purpose:      GoogleTest suite for TCP server
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/networking/tcp_server.h"
#include "hft/networking/tcp_client.h"
#include <thread>
#include <chrono>

using namespace hft::networking;

TEST(TCPServerTest, ServerStartup) {
    TCPServer server;
    EXPECT_TRUE(server.start(19999));
    server.stop();
}

TEST(TCPServerTest, ClientConnection) {
    TCPServer server;
    ASSERT_TRUE(server.start(19998));
    
    std::thread server_thread([&]() {
        server.accept_connection();
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TCPClient client;
    EXPECT_TRUE(client.connect("127.0.0.1", 19998));
    
    server_thread.join();
    
    client.disconnect();
    server.stop();
}

TEST(TCPServerTest, DataTransfer) {
    TCPServer server;
    ASSERT_TRUE(server.start(19997));
    
    constexpr char TEST_MESSAGE[] = "Hello from client!";
    std::string received_msg;
    
    std::thread server_thread([&]() {
        auto conn = server.accept_connection();
        if (conn >= 0) {
            char buffer[256];
            int bytes = server.receive(conn, buffer, sizeof(buffer));
            if (bytes > 0) {
                received_msg = std::string(buffer, bytes);
            }
        }
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TCPClient client;
    ASSERT_TRUE(client.connect("127.0.0.1", 19997));
    
    client.send(TEST_MESSAGE, sizeof(TEST_MESSAGE));
    
    server_thread.join();
    
    EXPECT_EQ(received_msg, TEST_MESSAGE);
    
    client.disconnect();
    server.stop();
}
