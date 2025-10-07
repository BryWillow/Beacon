#include "ItchMarketDataProcessor.h"
#include <iostream>

int main() {
  beacon::nsdq::itch::ItchMarketDataProcessor processor;

  processor.setCallback([](const beacon::nsdq::itch::AddOrderMessage &msg) {
    std::cout << "Received AddOrder: " << msg.OrderRefNum << "\n";
  });

  processor.run();
  return 0;
}