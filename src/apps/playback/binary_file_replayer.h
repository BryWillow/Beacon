/*
 * =============================================================================
 * Project:      Beacon
 * Application:  playback
 * Purpose:      Main playback orchestrator that coordinates message buffer,
 *               rules engine, and message sender for market data replay.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include "interfaces/IPlaybackMarketData.h" // CMake
#include "authorities/rules_engine.h"  // CMake
#include "message_buffer.h"
#include "playback_state.h"
#include <nlohmann/json.hpp>

namespace payback::replayer {

struct PlaybackConfig {
  std::string sender_type;
  std::string address;
  uint16_t port;
  uint8_t ttl;
  std::string file_path;
};

inline PlaybackConfig loadPlaybackConfig(const std::string& jsonPath) {
  std::ifstream f(jsonPath);
  nlohmann::json j;
  f >> j;
  PlaybackConfig cfg;
  cfg.sender_type = j.value("sender_type", "udp");
  cfg.address = j.value("address", "239.255.0.1");
  cfg.port = j.value("port", 12345);
  cfg.ttl = j.value("ttl", 1);
  cfg.file_path = j.value("file_path", "");
  // ...load other fields...
  return cfg;
}

class MarketDataPlayback {
  private:
    std::unique_ptr<IPlaybackMarketData> _sender;  
    MessageBuffer _buffer;
    RulesEngine _rulesEngine;
    PlaybackState _state;
    bool _loopForever = false;private:

public:
  MarketDataPlayback(std::unique_ptr<IPlaybackMarketData> sender)
    : _sender(std::move(sender)) {}

  MarketDataPlayback(IPlaybackMarketData* sender)
    : _sender(sender) {}

  // Load the binary market data file: NSDQ, NYSE, or CME.
  bool loadFile(const std::string& filePath) {
    return _buffer.load(filePath);
  }

  // Rules are both time and situational constraints.
  // You can add multiple rules to the playback engine.
  // E.g., ChaosRule, TimeWindowRule, SequenceRule.
  // Note that some rules may be VETO'd, e.g. queue the message.
  void addRule(std::unique_ptr<PlaybackRule> rule) {
    _rulesEngine.addRule(std::move(rule));
  }

  // Allow the file to keep replaying in a loop.
  void setLoopForever(bool loopForever) {
    _loopForever = loopForever;
  }

  // Run the playback.
  void run() {
    if (!_buffer.isLoaded()) {
      std::cerr << "Error: No file loaded\n";
      return;
    }

    printStartBanner();

    // Notify rules that playback is starting
    _rulesEngine.notifyPlaybackStart();

    auto startTime = std::chrono::steady_clock::now();

    // Main playback loopForever - loopForever forever if configured
    do {
      for (size_t i = 0; i < _buffer.size(); i++) {
        const char* message = _buffer.getMessage(i);

        // Evaluate rules to get decision
        auto decision = _rulesEngine.evaluate(i, message, _state);

        // Handle decision
        switch (decision.outcome) {
          case PlaybackRule::Outcome::DROP:
            _state.recordDropped();
            break;

          case PlaybackRule::Outcome::VETO:
            _state.recordQueued();  // Vetoed = held/queued
            break;

          case PlaybackRule::Outcome::SEND_NOW:
          case PlaybackRule::Outcome::CONTINUE:
          case PlaybackRule::Outcome::MODIFIED:
            // Apply any accumulated delay
            if (decision.accumulatedDelay.count() > 0) {
              std::this_thread::sleep_for(decision.accumulatedDelay);
            }
            // Send the message via the specified protocol.
            if (_sender->send(message, MessageBuffer::MESSAGE_SIZE)) {
              _state.recordSent();
            }
            break;
        }

        // Print periodic statistics
        if ((i + 1) % 10000 == 0 || i == _buffer.size() - 1) {
          printStats(i + 1);
        }
      }
    } while (_loopForever);

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Notify rules that playback has ended
    _rulesEngine.notifyPlaybackEnd();

    printEndBanner(duration);
  }

private:
  void printStartBanner() {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "  [5] <UDP Replayer> Starting...                                               \n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "      File:     " << _buffer.getFilePath() << "\n";
    std::cout << "      Messages: " << _buffer.size() << "\n";
    std::cout << "      Rules:    " << _rulesEngine.getRuleCount() << " configured\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "  [6] <UDP Replayer> Started                                                   \n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "      System Running...\n\n";
  }

  void printStats(size_t processed) {
    double pct = (100.0 * processed) / _buffer.size();
    size_t rate = _state.getCurrentRate();

    std::cout << "Progress: " << processed << "/" << _buffer.size()
              << " (" << std::fixed << std::setprecision(1) << pct << "%) "
              << "| Rate: " << rate << " msg/s "
              << "| Sent: " << _state.getTotalSent()
              << " | Dropped: " << _state.getMessagesDropped() << "\n";
  }

  void printEndBanner(std::chrono::milliseconds duration) {
    std::cout << "\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "                      MARKET DATA PLAYBACK - COMPLETE                          \n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n";
    std::cout << "  Duration:      " << duration.count() << " ms\n";
    std::cout << "  Total Sent:    " << _state.getTotalSent() << "\n";
    std::cout << "  Dropped:       " << _state.getMessagesDropped() << "\n";
    std::cout << "  Queued:        " << _state.getMessagesQueued() << "\n";

    double rate = _state.getTotalSent() / (duration.count() / 1000.0);
    std::cout << "  Average Rate:  " << std::fixed << std::setprecision(0)
              << rate << " msg/s\n";
    std::cout << "═══════════════════════════════════════════════════════════════════════════════\n\n";
  }
};
}