// ---------------------------------------------------------------------------
// File        : main_listener.cpp
// Description : Market Data Listener for NSDQ ITCH messages
// ---------------------------------------------------------------------------
#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <thread>
#include "json.hpp"
#include "nsdq/itch_message.h"
#include "nsdq/itch_udp_listener.h"
#include "spsc_ringbuffer.h"
#include "spsc_ringbuffer_consumer.h"

using json = nlohmann::json;

std::atomic<bool> g_stopFlag{false};
void signalHandler(int) { g_stopFlag.store(true); }

struct SimpleStrategy
{
    std::unordered_map<std::string, uint64_t> symbolCounts;
    void processMessage(const ItchMessage &msg)
    {
        if (msg.isAddOrder()) symbolCounts[msg.symbolStr()]++;
    }
    void printCounts() const
    {
        std::cout << "--- Message Counts ---\n";
        for (const auto &[sym, count] : symbolCounts) std::cout << sym << ": " << count << "\n";
        std::cout << "--------------------\n";
    }
};

struct ListenerConfig
{
    uint16_t udpPort{5555};
    size_t ringBufferSize{2048};
    int cpuCoreListener{0};
    int cpuCoreConsumer{1};
    std::chrono::seconds marketDataIdleTimeout{1800};
};

ListenerConfig loadListenerConfig(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + path);

    json configJson; file >> configJson;
    const auto &listenerJson = configJson["listener"];
    ListenerConfig cfg;
    cfg.udpPort = listenerJson.value("udp_port", 5555);
    cfg.ringBufferSize = listenerJson.value("mkt_data_buffer_size", 2048);
    cfg.cpuCoreListener = listenerJson.value("cpu_core_listener", 0);
    cfg.cpuCoreConsumer = listenerJson.value("cpu_core_consumer", 1);

    std::string timeoutStr = listenerJson.value("market_data_idle_timeout", "00:30:00");
    int hh, mm, ss;
    if (std::sscanf(timeoutStr.c_str(), "%d:%d:%d", &hh, &mm, &ss) == 3)
        cfg.marketDataIdleTimeout = std::chrono::hours(hh) + std::chrono::minutes(mm) + std::chrono::seconds(ss);

    return cfg;
}

int main()
{
    try
    {
        ListenerConfig cfg = loadListenerConfig("config.json");
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        SpScRingBuffer<ItchMessage, hft::DEFAULT_RING_BUFFER_CAPACITY> marketDataRingBuffer;
        auto listenerCallback = [&marketDataRingBuffer](const ItchMessage &msg)
        {
            while (!marketDataRingBuffer.tryPush(msg)) _mm_pause();
        };
        ItchUdpListener listener(cfg.udpPort, listenerCallback, cfg.cpuCoreListener);
        listener.start();

        SimpleStrategy strategy;
        auto consumerLambda = [&strategy](const ItchMessage &msg){ strategy.processMessage(msg); };
        auto consumer = make_ring_buffer_consumer(marketDataRingBuffer, consumerLambda);
        consumer.start(cfg.cpuCoreConsumer);

        auto lastMessageTime = std::chrono::steady_clock::now();
        while (!g_stopFlag.load(std::memory_order_relaxed))
        {
            if (!marketDataRingBuffer.empty()) lastMessageTime = std::chrono::steady_clock::now();
            if (std::chrono::steady_clock::now() - lastMessageTime > cfg.marketDataIdleTimeout)
            {
                std::cout << "[Listener] Market data idle timeout. Exiting.\n";
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        listener.stop();
        consumer.stop();
        strategy.printCounts();
        std::cout << "[Listener] Pipeline complete.\n";
    }
    catch (const std::exception &e) { std::cerr << "[Listener] Error: " << e.what() << "\n"; return 1; }
    return 0;
}
