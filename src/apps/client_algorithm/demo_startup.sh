#!/bin/bash
# Professional startup sequence demo

echo "Starting Beacon System Components..."
echo ""

# Clean up first
../../../kill_all.sh

sleep 1

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "                   BEACON SYSTEM STARTUP                          "
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Start matching engine
../../exchange_matching_engine/build/exchange_matching_engine 9000 &
ME_PID=$!
sleep 1

# Start market data playback
../../exchange_market_data_playback/build/exchange_market_data_playback \
    --config ../../exchange_market_data_playback/config_udp_slow.json \
    ../../exchange_market_data_generator/output.mdp > /tmp/md_playback.log 2>&1 &
MD_PID=$!
sleep 2

# Start algorithm
./build/algo_template 239.255.0.1 12345 127.0.0.1 9000 10

# Cleanup
echo ""
echo "Cleaning up..."
kill $ME_PID $MD_PID 2>/dev/null
wait 2>/dev/null

echo "Demo complete!"
