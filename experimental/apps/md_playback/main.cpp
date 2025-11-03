/**
 * @project   Beacon
 * @file      md_playback.cpp
 * @brief     Plays back market data messages for simulation purposes.
 * @author    Bryan Camp
 * @component Market Data Playback App
 * @details   This app plays back binary market data messages for testing.
 *            Options: debug(default)|release|all|clean|pipeline
 *            Each app also has its own out-of-source build script.
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <json/json.hpp>

namespace fs = std::filesystem;

// Stub: replace with your ITCH message validation
bool isITCHMessage(const uint8_t* msg, size_t len) {
    // TODO: Implement ITCH format validation
    return true;
}

// Wait for a process to signal readiness (stub: checks for "<name>.ready" file)
bool waitForProcessReady(const std::string& processName, int timeoutSeconds = 10) {
    std::string readyFile = processName + ".ready";
    int waited = 0;
    while (waited < timeoutSeconds) {
        if (std::ifstream(readyFile)) {
            std::cout << "[md_playback] " << processName << " is ready.\n";
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        waited++;
    }
    std::cerr << "[md_playback] Timeout waiting for " << processName << " to be ready.\n";
    return false;
}

/**
 * @brief Main entry point for md_playback.
 * Parses command-line arguments, validates input, and plays back market data messages.
 */
int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string configFile;
    bool waitForReady = false;

    // Simple arg parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--wait-for-ready") waitForReady = true;
        else if (arg == "--config" && i + 1 < argc) configFile = argv[++i];
        else inputFile = arg;
    }

    // Fallback to default config if not specified
    if (configFile.empty()) {
        configFile = "md_playback_default.json";
        std::cout << "[md_playback] No config specified, using default: " << configFile << "\n";
    }

    if (inputFile.empty()) {
        std::cerr << "Usage: md_playback [--wait-for-ready] [--config <config.json>] <input_file>\n";
        return 1;
    }

    // Load and parse .json configuration
    nlohmann::json config;
    std::ifstream cfg(configFile);
    if (!cfg) {
        std::cerr << "[md_playback] Error: Cannot open config file " << configFile << std::endl;
        return 1;
    }
    cfg >> config;
    cfg.close();
    // TODO: Use config values as needed

    try {
        if (waitForReady) {
            if (!waitForProcessReady("md_algo") || !waitForProcessReady("ex_match")) {
                std::cerr << "[md_playback] Required processes not ready. Exiting.\n";
                return 2;
            }
        }

        // Read entire file into memory
        std::ifstream in(inputFile, std::ios::binary);
        if (!in) {
            std::cerr << "[md_playback] Error: Cannot open file " << inputFile << std::endl;
            return 1;
        }
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        // Validate all messages before playback
        size_t offset = 0;
        while (offset < buffer.size()) {
            // TODO: Parse message length and boundaries
            size_t msgLen = /* parse message length */;
            const uint8_t* msg = buffer.data() + offset;

            if (!isITCHMessage(msg, msgLen)) {
                std::cerr << "[md_playback] Error: Non-ITCH message found. Exiting.\n";
                // TODO: Notify md_algo if needed
                return 3;
            }
            offset += msgLen;
        }

        std::cout << "[md_playback] All messages validated. Starting playback...\n";
        // TODO: Begin UDP playback of messages in buffer
        // ...playback logic...

    } catch (const std::exception& e) {
        std::cerr << "[md_playback] Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}