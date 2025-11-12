#include <gtest/gtest.h>
#include "market_data_playback.h"

TEST(BinaryTest, ValidFilesParse) {
  ASSERT_TRUE(MarketDataPlayback::testParse("test_data/nsdq_valid.bin"));
  ASSERT_TRUE(MarketDataPlayback::testParse("test_data/nyse_valid.bin"));
  ASSERT_TRUE(MarketDataPlayback::testParse("test_data/cme_valid.bin"));
}

TEST(BinaryTest, MalformedFilesFail) {
  ASSERT_FALSE(MarketDataPlayback::testParse("test_data/nsdq_malformed.bin"));
  ASSERT_FALSE(MarketDataPlayback::testParse("test_data/nyse_malformed.bin"));
  ASSERT_FALSE(MarketDataPlayback::testParse("test_data/cme_malformed.bin"));
}
