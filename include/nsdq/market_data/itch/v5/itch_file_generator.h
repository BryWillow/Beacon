// ---------------------------------------------------------------------------
// @file        itch_file_generator.h
// Project     : Beacon
// Component   : NSDQ ITCH Utilities
// Description : Generates capture files with random ITCH 5.0 messages
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "include/hft/networking/hton_utils.h"
#include "itch_messages.h"

namespace bhnetwork = beacon::hft::networking;

namespace beacon::nsdq::market_data::itch {

/**
 * @enum ItchMessageKind
 * @brief Enumerates valid ITCH message types for generation
 */
enum class ItchMessageKind {
    Invalid,  ///< Placeholder / uninitialized
    AddOrder,
    Trade,
    Cancel,
    Delete
};
/**
 * @class ItchFileGenerator
 * @brief Generates capture files with true ITCH 5.0 messages
 */
class ItchFileGenerator {
   public:
    /**
     * @brief Generate a capture file with ITCH 5.0 messages
     * @param fileName Name of the output file
     * @param numMessages Number of messages to generate
     */
    static void Generate(const std::string& fileName, size_t numMessages) {
        std::filesystem::path dataDir("data");
        if (!std::filesystem::exists(dataDir))
            std::filesystem::create_directories(dataDir);

        std::filesystem::path filePath = dataDir / fileName;
        std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
        if (!outFile)
            throw std::runtime_error("Failed to open capture file");

        std::mt19937_64 rng(0xDEADBEEF);
        std::uniform_int_distribution<uint64_t> orderIdDist(1, 1'000'000);
        std::uniform_int_distribution<uint32_t> priceDist(100, 10'000);
        std::uniform_int_distribution<uint32_t> qtyDist(1, 1'000);
        std::uniform_int_distribution<uint8_t> sideDist(0, 1);
        std::uniform_int_distribution<size_t> kindDist(
            0, 3);  // 0..3 -> AddOrder, Trade, Cancel, Delete

        const std::array<ItchMessageKind, 4> kinds{ItchMessageKind::AddOrder,
                                                   ItchMessageKind::Trade, ItchMessageKind::Cancel,
                                                   ItchMessageKind::Delete};

        uint64_t tsNs = 0;
        uint64_t seqNum = 0;

        for (size_t i = 0; i < numMessages; ++i) {
            ItchMessageKind kind = kinds[kindDist(rng)];

            switch (kind) {
                case ItchMessageKind::AddOrder: {
                    AddOrderMessage msg{};
                    msg.orderRefNum = bhnetwork::htonll(orderIdDist(rng));
                    std::memset(msg.stock, ' ', 8);
                    std::strncpy(msg.stock, "AAPL", 4);
                    msg.shares = htonl(qtyDist(rng));
                    msg.price = htonl(priceDist(rng));
                    msg.side = sideDist(rng) ? 'B' : 'S';
                    outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
                    break;
                }
                case ItchMessageKind::Trade: {
                    TradeMessage msg{};
                    msg.orderRefNum = bhnetwork::htonll(seqNum++);
                    std::memset(msg.stock, ' ', 8);
                    std::strncpy(msg.stock, "AAPL", 4);
                    msg.shares = htonl(qtyDist(rng));
                    msg.price = htonl(priceDist(rng));
                    outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
                    break;
                }
                case ItchMessageKind::Cancel: {
                    OrderCancelMessage msg{};
                    msg.orderRefNum = bhnetwork::htonll(orderIdDist(rng));
                    msg.canceledShares = htonl(qtyDist(rng));
                    outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
                    break;
                }
                case ItchMessageKind::Delete: {
                    OrderDeleteMessage msg{};
                    msg.orderRefNum = bhnetwork::htonll(orderIdDist(rng));
                    outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
                    break;
                }
                default:
                    break;  // should never occur
            }

            tsNs += 1 + (rng() % 100);  // simulate spacing
        }
    }
};

}  // namespace beacon::nsdq::market_data::itch
