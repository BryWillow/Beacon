/**
 * @file message_generator.h
 * @brief Message generation and config parsing for md_generator.
 *
 * @note
 * Project Checklist:
 *   - Use concise, descriptive executable names (e.g., md_generator, md_server, md_trader).
 *   - To add new parameters to the config:
 *       - Add the new field to the relevant struct (e.g., SymbolParameters, PriceRange).
 *       - Update the JSON parsing logic in MessageGenerator::parseJson() (see .cpp).
 *       - Add validation for the new parameter in MessageGenerator::validateConfig() (recommended).
 *       - Document the new parameter in the config file and usage/help output.
 *
 * Validation should be performed immediately after parsing, in a dedicated method
 * (e.g., validateConfig()), to ensure all config values are correct before use.
 *
 * @note
 * The MessageGenerator constructor loads and validates the config file.
 * If the config is malformed, it throws an exception with a descriptive error message.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <json/json.hpp>

constexpr uint32_t ITCH_MAX_ORDER_QUANTITY = 1000000;
constexpr double ITCH_MAX_ORDER_PRICE = 429496.7295; // ITCH price field is uint32_t, max value is 4294967295 ticks (429496.7295 dollars)

struct QuantityRange {
  uint32_t min_quantity;
  uint32_t max_quantity;
  double weight;
};

struct PriceRange {
  double min_price;
  double max_price;
  double weight;
};

struct SymbolParameters {
  std::string symbol;
  std::vector<PriceRange> price_ranges;
  std::vector<QuantityRange> quantity_ranges;
  // ...add more fields as needed...
};

class MessageGenerator {
public:
  /**
   * @brief Construct a MessageGenerator and load/validate config.
   * @param configPath Path to the config file (.json).
   * @throws std::runtime_error if config is malformed or validation fails.
   *
   * The constructor loads the config file and validates its content.
   * If the config is malformed or validation fails, an exception is thrown
   * with a descriptive error message.
   */
  explicit MessageGenerator(const std::string& configPath);

  // Delete copy and move operations to prevent accidental copying
  MessageGenerator(const MessageGenerator&) = delete;
  MessageGenerator& operator=(const MessageGenerator&) = delete;
  MessageGenerator(MessageGenerator&&) = delete;
  MessageGenerator& operator=(MessageGenerator&&) = delete;

  void generateMessages(const std::string& outputPath, size_t numMessages);
  void printStats() const;
  size_t getMessageCount() const { return _message_count; }

private:
  const std::vector<SymbolParameters> _symbols;
  const size_t _message_count = 10000; // default if not specified

  void parseJson(const nlohmann::json& j);
  void validateConfig(); // Called after parsing, throws on error
  // ...other private helpers...
};