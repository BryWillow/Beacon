#include "nyse/execution/nyse_message_types.h"

int main() {
    NyseOrderEntryMessage msg = {};
    msg.clientOrderId = 12345;
    strncpy(msg.symbol, "IBM", sizeof(msg.symbol));
    msg.shares = 100;
    msg.price = 1450000;
    msg.side = 'B';
    msg.orderType = 'L';
    msg.tif = 'G';
    // ...other fields...

    return 0;
}
