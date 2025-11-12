#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>

TEST(ConfigTest, ConfigFileExists) {
  std::ifstream f("config/playback/default.json");
  ASSERT_TRUE(f.good());
}

TEST(ConfigTest, ConfigFileParses) {
  std::ifstream f("config/playback/default.json");
  nlohmann::json j;
  ASSERT_NO_THROW(f >> j);
}
