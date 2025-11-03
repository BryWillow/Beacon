/**
 * @project   Beacon
 * @file      md_playback.cpp
 * @brief     Reads market data file and plays back ITCH messages.
 * @author    Bryan Camp
 * @component Market Data Playback App
 * @details   This app reads a file of market data messages and plays them back
 *            to the system. It checks that all messages are in the ITCH format
 *            and notifies md_algo if any non-ITCH messages are found.
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "nsdq/market_data/itch/messages_active.h" // ITCH message definitions

// Function to check if a message is in ITCH format
bool isITCHMessage(const uint8_t* msg, size_t len) {
    // Implement ITCH format validation here.
    // For example, check message type, length, etc.
    // Return true if valid, false otherwise.
    // ...your ITCH validation logic...
    return true; // stub
}

// Function to notify md_algo about an issue
void notifyMdAlgo(const std::string& reason) {
    // TODO: Implement notification to md_algo (e.g., via IPC, file, socket)
    std::cerr << "[md_playback] Notifying md_algo: " << reason << std::endl;
}

int main(int argc, char* argv[]) {
    // ...parse args, get input filename...
    std::string inputFile = "market_data.dat"; // example

    // Read the entire file into memory before playback begins
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        std::cerr << "[md_playback] Error: Cannot open file " << inputFile << std::endl;
        return 1;
    }

    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // Validate all messages before starting playback
    size_t offset = 0;
    while (offset < buffer.size()) {
        // Parse next message (assume fixed size or parse length from header)
        size_t msgLen = /* parse message length */;
        const uint8_t* msg = buffer.data() + offset;

        if (!isITCHMessage(msg, msgLen)) {
            notifyMdAlgo("Non-ITCH message encountered");
            std::cerr << "[md_playback] Error: Non-ITCH message found. Exiting.\n";
            return 2;
        }

        offset += msgLen;
    }

    // Only begin UDP playback after all messages are validated and loaded in memory
    // ...broadcast messages via UDP...

    return 0;
}