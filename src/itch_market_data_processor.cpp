// ---------------------------------------------------------------------------
// File        : itch_market_data_processor.cpp
// Project     : Beacon
// Component   : NSDQ ITCH 5.0
// Description : Market Data Processor for NSDQ ITCH feeds
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#include "itch_market_data_processor.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace beacon::nsdq::itch {
/// @brief Constructs a real-time ITCH market data processor.
/// @param cpuCore The CPU core on which this processor should run
ItchMarketDataProcessor::ItchMarketDataProcessor(
    int cpuCore = constants::NO_CPU_PINNING)
    : _cpuCore(cpuCore),
      _consumer(_ringBuffer, [this](const AddOrderMessage &msg) {
        if (_callback)
          _callback(msg);
      }) {}

ItchMarketDataProcessor::~ItchMarketDataProcessor() { stop(); }

void ItchMarketDataProcessor::run() {
  _stop.store(false, std::memory_order_relaxed);
  _consumer.start();
  while (!_stop.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  _consumer.stop();
}

void ItchMarketDataProcessor::stop() {
  _stop.store(true, std::memory_order_relaxed);
}

void ItchMarketDataProcessor::setCallback(Callback cb) {
  _callback = std::move(cb);
}

} // namespace beacon::nsdq::itch