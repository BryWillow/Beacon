/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Main entry point for market data playback tool. Loads binary
 *               market data files and replays them with configurable timing,
 *               burst patterns, and chaos testing rules.
 * Author:       Bryan Camp
 * =============================================================================
 */

#include "market_data_playback.h"
#include "message_sender.h"
#include "rules/common_rules.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;

// Create playback rules from configuration
void configureRules(MarketDataPlayback& playback, const nlohmann::json& config) {
    std::string mode = config.value("mode", "continuous");
    
    if (mode == "burst") {
        size_t burstSize = config.value("burst_size", 5000);
        int intervalMs = config.value("burst_interval_ms", 100);
        playback.addRule(std::make_unique<BurstRule>(
            burstSize, std::chrono::milliseconds(intervalMs)));
        std::cout << "Configured: Burst mode (" << burstSize 
                  << " msgs every " << intervalMs << "ms)\n";
    }
    else if (mode == "continuous") {
        size_t rate = config.value("rate_msgs_per_sec", 10000);
        playback.addRule(std::make_unique<ContinuousRule>(rate));
        std::cout << "Configured: Continuous mode (" << rate << " msgs/sec)\n";
    }
    else if (mode == "wave") {
        int periodMs = config.value("period_ms", 10000);
        size_t minRate = config.value("min_rate", 1000);
        size_t maxRate = config.value("max_rate", 100000);
        playback.addRule(std::make_unique<WaveRule>(
            std::chrono::milliseconds(periodMs), minRate, maxRate));
        std::cout << "Configured: Wave mode (" << minRate << "-" << maxRate 
                  << " msgs/sec, " << periodMs << "ms period)\n";
    }
    
    // Add speed factor if specified
    if (config.contains("speed_factor")) {
        double speedFactor = config["speed_factor"];
        playback.addRule(std::make_unique<SpeedFactorRule>(speedFactor));
        std::cout << "Configured: Speed factor " << speedFactor << "x\n";
    }
    
    // Add rate limit if specified (safety rule)
    if (config.contains("max_rate_limit")) {
        size_t maxRate = config["max_rate_limit"];
        playback.addRule(std::make_unique<RateLimitRule>(maxRate));
        std::cout << "Configured: Rate limit " << maxRate << " msgs/sec (SAFETY)\n";
    }
    
    // Add packet loss if specified (chaos rule)
    if (config.contains("packet_loss_rate")) {
        double lossRate = config["packet_loss_rate"];
        playback.addRule(std::make_unique<PacketLossRule>(lossRate));
        std::cout << "Configured: Packet loss " << (lossRate * 100) << "%\n";
    }
    
    // Add jitter if specified (chaos rule)
    if (config.contains("max_jitter_us")) {
        long jitterUs = config["max_jitter_us"];
        playback.addRule(std::make_unique<JitterRule>(
            std::chrono::microseconds(jitterUs)));
        std::cout << "Configured: Jitter up to " << jitterUs << " microseconds\n";
    }
}

void printUsage() {
    std::cout << "Usage: exchange_market_data_playback [--config <config.json>] <input_file>\n\n";
    std::cout << "Arguments:\n";
    std::cout << "  <input_file>         Binary market data file (.itch, .pillar, .mdp)\n";
    std::cout << "  --config <file>      Configuration file (optional)\n\n";
    std::cout << "Example:\n";
    std::cout << "  ./exchange_market_data_playback output.itch\n";
    std::cout << "  ./exchange_market_data_playback --config burst.json output.mdp\n";
}

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string configFile;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n\n";
        printUsage();
        return 1;
    }

    // Load configuration
    nlohmann::json config;
    if (!configFile.empty()) {
        std::ifstream cfg(configFile);
        if (!cfg) {
            std::cerr << "Error: Cannot open config file: " << configFile << "\n";
            return 1;
        }
        cfg >> config;
        std::cout << "Loaded configuration from: " << configFile << "\n";
    } 
    else {
        // Default configuration
        config = {
            {"mode", "continuous"},
            {"rate_msgs_per_sec", 10000}
        };
        std::cout << "Using default configuration (continuous 10K msgs/sec)\n";
    }

    try {
        // Create sender based on configuration
        std::unique_ptr<MessageSender> sender;
        std::string outputType = config.value("output", "console");
        
        if (outputType == "tcp") {
            uint16_t port = config.value("tcp_port", 12345);
            sender = std::make_unique<TcpSender>(port);
            std::cout << "[TCP] Ready to send data to connected client\n";
        }
        else if (outputType == "udp_multicast") {
            std::string multicastAddr = config.value("multicast_address", "239.255.0.1");
            uint16_t port = config.value("multicast_port", 12345);
            uint8_t ttl = config.value("multicast_ttl", 1);
            sender = std::make_unique<UdpMulticastSender>(multicastAddr, port, ttl);
        }
        else if (outputType == "file") {
            std::string outputFile = config.value("output_file", "output_replay.bin");
            sender = std::make_unique<FileSender>(outputFile);
        }
        else if (outputType == "null") {
            sender = std::make_unique<NullSender>();
            std::cout << "Using null sender (dry-run mode)\n";
        }
        else {
            // Default to console
            sender = std::make_unique<ConsoleSender>();
            std::cout << "Using console sender (debug mode)\n";
        }
        
        // Create playback engine
        MarketDataPlayback playback(std::move(sender));
        
        // Load file
        if (!playback.loadFile(inputFile)) {
            return 1;
        }
        
        // Configure rules from config
        configureRules(playback, config);
        
        // Check if we should loop forever
        bool loopForever = config.value("loop_forever", false);
        if (loopForever) {
            playback.setLoopForever(true);
            std::cout << "Configured: Loop forever (continuous streaming)\n";
        }
        
        // Run playback
        playback.run();

    } 
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}