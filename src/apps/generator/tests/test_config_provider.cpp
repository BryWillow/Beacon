#include <gtest/gtest.h>
#include "../config_provider.h"

using namespace beacon::market_data_generator::config;

TEST(ConfigProviderTest, LoadsValidConfig) {
  ConfigProvider provider("nsdq", "output.bin");
  EXPECT_TRUE(provider.loadConfig("test_valid_config.json"));
  EXPECT_GT(provider.getMessageCount(), 0);
  EXPECT_FALSE(provider.getSymbolsForGeneration().empty());
}

TEST(ConfigProviderTest, FailsOnMissingFile) {
  ConfigProvider provider("nsdq", "output.bin");
  EXPECT_THROW(provider.loadConfig("nonexistent.json"), std::runtime_error);
}

TEST(ConfigProviderTest, FailsOnMalformedConfig) {
  ConfigProvider provider("nsdq", "output.bin");
  EXPECT_THROW(provider.loadConfig("test_malformed_config.json"), std::runtime_error);
}

TEST(ConfigProviderTest, ValidatesExchangeType) {
  ConfigProvider provider("invalid", "output.bin");
  EXPECT_THROW(provider.loadConfig("test_valid_config.json"), std::runtime_error);
}

TEST(ConfigProviderTest, ValidatesNumMessages) {
  ConfigProvider provider("nsdq", "output.bin");
  EXPECT_THROW(provider.loadConfig("test_zero_messages_config.json"), std::runtime_error);
}

// ...add more tests for trade_probability, flush_interval, spread_percentage, etc...
