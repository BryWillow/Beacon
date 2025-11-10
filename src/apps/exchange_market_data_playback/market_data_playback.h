/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Main playback orchestrator that coordinates message buffer,
 *               rules engine, and message sender for market data replay.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include "message_buffer.h"
#include "message_sender.h"
#include "playback_state.h"
#include "rules_engine.h"

class MarketDataPlayback {
public:
    MarketDataPlayback(std::unique_ptr<MessageSender> sender)
        : _sender(std::move(sender)) {}
    
    // Load a binary market data file
    bool loadFile(const std::string& filePath) {
        return _buffer.load(filePath);
    }
    
    // Add a rule to the playback engine
    void addRule(std::unique_ptr<PlaybackRule> rule) {
        _rulesEngine.addRule(std::move(rule));
    }
    
    // Set whether to loop the file forever
    void setLoopForever(bool loop) {
        _loopForever = loop;
    }
    
    // Run the playback
    void run() {
        if (!_buffer.isLoaded()) {
            std::cerr << "Error: No file loaded\n";
            return;
        }
        
        printStartBanner();
        
        // Notify rules that playback is starting
        _rulesEngine.notifyPlaybackStart();
        
        auto startTime = std::chrono::steady_clock::now();
        
        // Main playback loop - loop forever if configured
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
                        
                        // Send the message
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
    
    MessageBuffer _buffer;
    RulesEngine _rulesEngine;
    PlaybackState _state;
    std::unique_ptr<MessageSender> _sender;
    bool _loopForever = false;
};
