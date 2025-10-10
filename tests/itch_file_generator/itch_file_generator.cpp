// ---------------------------------------------------------------------------
// @file        itch_file_generator_test.cpp
// @project     Beacon
// @component   Unit Tests / NSDQ Pipeline
// @description Verifies NSDQ ITCH file generation
// @author      Bryan Camp
// ---------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <filesystem>
#include "nsdq/market_data/itch/stubs/itch_file_generator.h"

namespace beaconmd = beacon::nsdq::market_data::itch;
namespace fs = std::filesystem;

class ItchFileGeneratorTest : public ::testing::Test {
 protected:
  fs::path testDir;

  void SetUp() override {
    testDir = fs::temp_directory_path() / "beacon_itch_test";
    fs::create_directories(testDir);
  }

  void TearDown() override {
    fs::remove_all(testDir);
  }
};

TEST_F(ItchFileGeneratorTest, GeneratesFileWithCorrectSize) {
  const size_t numMessages = 1000;
  fs::path filePath = testDir / "test.itch";

  beaconmd::ItchFileGenerator::Generate(filePath.string(), numMessages);

  ASSERT_TRUE(fs::exists(filePath));

  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  ASSERT_TRUE(file.is_open());

  // Simple check: file size >= numMessages * smallest possible ITCH message
  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  ASSERT_GE(size, static_cast<std::streamsize>(numMessages * sizeof(beaconmd::AddOrderMessage)));
}
