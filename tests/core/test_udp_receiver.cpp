// =============================================================================
// Project:      Beacon Tests
// File:         test_udp_receiver.cpp
// Purpose:      GoogleTest suite for UDP receiver
// Author:       Bryan Camp
// =============================================================================

#include <gtest/gtest.h>
#include "hft/networking/udp_receiver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

using namespace hft::networking;

TEST(UDPReceiverTest, MulticastSetup) {
    UDPReceiver receiver;
    EXPECT_TRUE(receiver.setup("239.255.0.100", 12346));
    receiver.close();
}

TEST(UDPReceiverTest, ReceiveData) {
    UDPReceiver receiver;
    ASSERT_TRUE(receiver.setup("239.255.0.101", 12347));
    
    constexpr char TEST_MSG[] = "UDP Test Message";
    
    // Send data from another thread
    std::thread sender([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_GE(sock, 0);
        
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(12347);
        inet_pton(AF_INET, "239.255.0.101", &addr.sin_addr);
        
        sendto(sock, TEST_MSG, sizeof(TEST_MSG), 0,
               (sockaddr*)&addr, sizeof(addr));
        
        close(sock);
    });
    
    char buffer[256];
    int bytes = receiver.receive(buffer, sizeof(buffer));
    
    sender.join();
    
    EXPECT_GT(bytes, 0);
    if (bytes > 0) {
        EXPECT_STREQ(buffer, TEST_MSG);
    }
    
    receiver.close();
}
