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

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "market_data_playback.h"
#include <message_senders/message_sender_factory.h>
#include <playback_authorities/rules_engine.h>
#include <atomic>
#include <thread>
#include <chrono>

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

nlohmann::json load_json(const std::string& path) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("Config file not found: " + path);
  nlohmann::json j;
  f >> j;
  return j;
}

int main(int argc, char* argv[]) {
  std::string inputFile;
  std::string configFile;

  std::string configPath = "config/playback/default.json";
  if (argc > 1) configPath = argv[1];

  nlohmann::json defaultConfig = load_json(configPath);

  // Load sender config
  nlohmann::json senderConfig = load_json("config/playback/" + defaultConfig["sender_config"].get<std::string>());
  std::string senderType = senderConfig.value("type", "udp");
  std::string address = senderConfig.value("address", "239.255.0.1");
  uint16_t port = senderConfig.value("port", 12345);
  uint8_t ttl = senderConfig.value("ttl", 1);
  auto sender = market_data_playback::createSender(senderType, address, port, ttl);

  // Load advisors
  std::vector<nlohmann::json> advisorConfigs;
  for (const auto& advPath : defaultConfig["advisors"]) {
    advisorConfigs.push_back(load_json("config/playback/" + advPath.get<std::string>()));
  }
  // Instantiate advisors as needed

  // Load authorities (rules)
  market_data_playback::playback_authorities::RulesEngine rulesEngine;
  for (const auto& authPath : defaultConfig["authorities"]) {
    nlohmann::json ruleConfig = load_json("config/playback/" + authPath.get<std::string>());
    std::string type = ruleConfig.value("type", "");
    // ...instantiate rules based on type and config...
    // Example for burst_rule:
    if (type == "burst_rule") {
      size_t burstSize = ruleConfig.value("burst_size", 100);
      int burstIntervalMs = ruleConfig.value("burst_interval_ms", 100);
      rulesEngine.addRule(std::make_unique<market_data_playback::playback_authorities::BurstRule>(burstSize, std::chrono::milliseconds(burstIntervalMs)));
    }
    // ...repeat for other rule types...
  }

    // Parse arguments
    bool printSummary = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else if (arg == "--summary") {
            printSummary = true;
        } else {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n\n";
        printUsage();
        return 1;
    }

    // Create playback engine
    MarketDataPlayback playback(sender.get() /*, advisors, rulesEngine, ...*/);
    
    // Load file
    if (!playback.loadFile(inputFile)) {
        return 1;
    }
    
    // Configure rules from config
    configureRules(playback, j);
    
    // Check if we should loop forever
    bool loopForever = j.value("loop_forever", false);
    if (loopForever) {
        playback.setLoopForever(true);
        std::cout << "Configured: Loop forever (continuous streaming)\n";
    }
    
    std::atomic<bool> running{true};
    std::thread summaryThread;
    if (printSummary) {
      summaryThread = std::thread([&]() {
        while (running) {
          std::this_thread::sleep_for(std::chrono::seconds(5));
          std::cout << "[SUMMARY] Messages sent: " << playback.getMessagesSent() << std::endl;
        }
      });
    }

    // Run playback
    playback.run();
    running = false;
    if (printSummary && summaryThread.joinable()) summaryThread.join();
    std::cout << "[SUMMARY] Final messages sent: " << playback.getMessagesSent() << std::endl;
    return 0;
}