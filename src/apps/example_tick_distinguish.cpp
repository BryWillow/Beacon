#include "beacon_common/beacon_common.h"
#include <cstdio>

void printTick(const MarketDataTick& tick) {
    printf("Tick: symbol=%.8s price=%u shares=%u type=%c\n", tick.symbol, tick.price, tick.shares, tick.type);
}

int main() {
    MarketDataTick tick = {};
    strncpy(tick.symbol, "AAPL", sizeof(tick.symbol));
    tick.price = 1750000;
    tick.shares = 100;
    tick.type = 'A'; // Add order

    printTick(tick);

    // ...add more ITCH-compatible ticks as needed...

    return 0;
}
