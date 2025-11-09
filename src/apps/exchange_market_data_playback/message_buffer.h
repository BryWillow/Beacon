/*
 * =============================================================================
 * Project:      Beacon
 * Application:  exchange_market_data_playback
 * Purpose:      Loads and validates binary market data files into memory for
 *               fast, consistent playback without disk I/O delays.
 * Author:       Bryan Camp
 * =============================================================================
 */

#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class MessageBuffer {
public:
    static constexpr size_t MESSAGE_SIZE = 33;  // ITCH messages are 33 bytes (packed structs)
    
    // Load entire file into memory
    bool load(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open file: " << filePath << "\n";
            return false;
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Validate: must be multiple of 32 bytes
        if (fileSize % MESSAGE_SIZE != 0) {
            std::cerr << "Error: File size (" << fileSize 
                      << " bytes) is not a multiple of " << MESSAGE_SIZE << " bytes\n";
            return false;
        }
        
        _messageCount = fileSize / MESSAGE_SIZE;
        _messages.resize(_messageCount);
        
        // Load entire file into memory
        for (size_t i = 0; i < _messageCount; i++) {
            file.read(_messages[i].data(), MESSAGE_SIZE);
            if (!file) {
                std::cerr << "Error: Failed to read message " << i << "\n";
                return false;
            }
        }
        
        _filePath = filePath;
        std::cout << "Loaded " << _messageCount << " messages (" 
                  << fileSize << " bytes) from " << filePath << "\n";
        return true;
    }
    
    // Get a specific message by index
    const char* getMessage(size_t index) const {
        if (index >= _messageCount) {
            return nullptr;
        }
        return _messages[index].data();
    }
    
    // Get total number of messages
    size_t size() const { return _messageCount; }
    
    // Get source file path
    const std::string& getFilePath() const { return _filePath; }
    
    // Check if buffer is loaded
    bool isLoaded() const { return _messageCount > 0; }
    
private:
    std::vector<std::array<char, MESSAGE_SIZE>> _messages;
    size_t _messageCount = 0;
    std::string _filePath;
};
