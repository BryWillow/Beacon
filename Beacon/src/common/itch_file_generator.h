// ---------------------------------------------------------------------------
// File        : itch_file_generator.h
// Project     : Beacon
// Component   : Utilities
// Description : Generates capture files with random ITCH messages
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <fstream>
#include <random>
#include <string>
#include <filesystem>
#include "itch_message.h"
#include "captured_message.h"

/**
 * @class ItchFileGenerator
 * @brief Utility to generate capture files containing random ITCH messages.
 *
 * Notes:
 * - Creates a `data/` directory if it does not exist.
 * - Uses a fixed RNG seed for deterministic output.
 * - Generates `CapturedMessage` structs with incremental timestamps.
 */
class ItchFileGenerator {
public:
    /**
     * @brief Generate a capture file with random messages.
     * @param fileName Name of the output file (inside `data/` directory)
     * @param numMessages Number of messages to generate
     *
     * Tricky parts:
     * - Uses uniform distributions for order ID, price, quantity, and side.
     * - Timestamps increment by 1-100 ns per message to simulate realistic spacing.
     */
    static void Generate(const std::string& fileName, size_t numMessages) {
        std::filesystem::path dataDir("data");
        if (!std::filesystem::exists(dataDir)) 
            std::filesystem::create_directories(dataDir);

        std::filesystem::path filePath = dataDir / fileName;
        std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
        if (!outFile) throw std::runtime_error("Failed to open capture file");

        std::mt19937_64 rng(0xDEADBEEF);                        // deterministic seed
        std::uniform_int_distribution<uint64_t> orderIdDist(1, 1000000);
        std::uniform_int_distribution<uint32_t> priceDist(100, 10000);
        std::uniform_int_distribution<uint32_t> qtyDist(1, 1000);
        std::uniform_int_distribution<uint8_t> sideDist(0, 1);

        uint64_t tsNs   = 0;
        uint64_t seqNum = 0;

        for (size_t i = 0; i < numMessages; ++i) {
            ItchMessage msg{};
            msg.orderId       = orderIdDist(rng);
            msg.price         = priceDist(rng);
            msg.size          = qtyDist(rng);
            msg.sequenceNumber= ++seqNum;
            msg.tsNanoSeconds = tsNs;
            msg.side          = (sideDist(rng) == 0 ? Side::Buy : Side::Sell);
            msg.type          = ItchMsgType::AddOrder;

            tsNs += 1 + (rng() % 100);  // simulate spacing

            CapturedMessage capMsg{};
            capMsg.msg  = msg;
            capMsg.tsNs = tsNs;

            outFile.write(reinterpret_cast<const char*>(&capMsg), sizeof(capMsg));
        }
    }
};