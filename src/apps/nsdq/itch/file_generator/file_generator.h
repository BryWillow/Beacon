// ---------------------------------------------------------------------------
// @file        nsdq_itch_file_generator.h
// Project     : Beacon
// Component   : NSDQ ITCH File Generator
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <random>
#include <cstdint>
#include <string>

#include "nsdq/market_data/itch/current/messages_active.h"
#include "hft/networking/hton_utils.h"

namespace beaconmd = beacon::nsdq::market_data::itch;
namespace beaconnetwork = beacon::hft::networking;

constexpr size_t numMessages = 10000;

class ItchFileGenerator {

  public:

    void Generate(const std::string& fileName, size_t numMessages) {

      // beacon/data/ should always exist, but just in case.
      std::filesystem::path topLevelDir("data");
      if (!std::filesystem::exists(topLevelDir)) {
        std::filesystem::create_directories(topLevelDir);
      }

      // If this file already exists, simply overwrite it for now.
      std::filesystem::path filePath = topLevelDir / fileName;
      std::ofstream outFile(filePath, std::ios::binary | std::ios::trunc);
      if (!outFile) {
        throw std::runtime_error("Failed to open capture file: " + filePath.string());
      }

      std::mt19937_64 rng(0xDEADBEEF);
      std::uniform_int_distribution<uint64_t> orderIdDist(0, 1000000);
      std::uniform_int_distribution<uint32_t> priceDist(100, 10000);
      std::uniform_int_distribution<uint32_t> qtyDist(1, 1000);
      std::uniform_int_distribution<uint8_t> sideDist(0, 1);
      std::uniform_int_distribution<size_t> kindDist( 0, 3);  // 0..3 -> AddOrder, Trade, Cancel, Delete

      const std::array<beaconmd::ItchMessageKind, 4> kinds{
        beaconmd::ItchMessageKind::AddOrder,
        beaconmd::ItchMessageKind::Trade,
        beaconmd::ItchMessageKind::Cancel,
        beaconmd::ItchMessageKind::Delete};

      uint64_t tsNs = 0;
      uint64_t seqNum = 0;

      for (size_t i = 0; i < numMessages; ++i) {
        beaconmd::ItchMessageKind kind = kinds[kindDist(rng)];
        switch (kind) {
          case beaconmd::ItchMessageKind::AddOrder: {
            beaconmd::AddOrderMessage msg{};
            msg.orderRefNum = beaconnetwork::htonll(orderIdDist(rng));
            std::memset(msg.stock, ' ', 8);
            std::strncpy(msg.stock, "AAPL", 4);
            msg.shares = htonl(qtyDist(rng));
            msg.price = htonl(priceDist(rng));
            msg.side = sideDist(rng) ? 'B' : 'S';
            outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
            break;
          }
          case beaconmd::ItchMessageKind::Trade: {
            beaconmd::TradeMessage msg{};
            msg.orderRefNum = beaconnetwork::htonll(seqNum++);
            std::memset(msg.stock, ' ', 8);
            std::strncpy(msg.stock, "AAPL", 4);
            msg.shares = htonl(qtyDist(rng));
            msg.price = htonl(priceDist(rng));
            outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
            break;
          }
          case beaconmd::ItchMessageKind::Cancel: {
            beaconmd::OrderCancelMessage msg{};
            msg.orderRefNum = beaconnetwork::htonll(orderIdDist(rng));
            msg.canceledShares = htonl(qtyDist(rng));
            outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
            break;
          }
          case beaconmd::ItchMessageKind::Delete: {
            beaconmd::OrderDeleteMessage msg{};
            msg.orderRefNum = beaconnetwork::htonll(orderIdDist(rng));
            outFile.write(reinterpret_cast<const char*>(&msg), sizeof(msg));
            break;
          }
          default:
            break;
        }
        tsNs += 1 + (rng() % 100);  // simulate spacing
      }
    }
};