#include "cme/execution/cme_message_types.h"

int main() {
    CmeOrderEntryMessage msg = {};
    msg.clientOrderId = 123456789;
    strncpy(msg.symbol, "ESZ2025", sizeof(msg.symbol));
    msg.quantity = 10;
    msg.price = 450000; // $45.0000
    msg.side = 'B';
    msg.orderType = 'L';
    msg.tif = 'G';

    printCmeOrderEntryMessage(msg);

    return 0;
}
