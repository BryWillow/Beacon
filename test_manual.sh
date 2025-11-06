#!/bin/bash
# Quick manual test - run each component in foreground to see output

echo "Starting manual test..."
echo ""
echo "Press Ctrl+C to stop at any time"
echo ""

# Kill any existing
pkill -f exchange_matching_engine || true
pkill -f exchange_market_data_playback || true
pkill -f algo_template || true
sleep 1

echo "[1] Starting Matching Engine in background..."
src/apps/exchange_matching_engine/build/exchange_matching_engine 9000 &
ME_PID=$!
sleep 2

echo ""
echo "[2] Starting Market Data Playback in background..."
src/apps/exchange_market_data_playback/build/exchange_market_data_playback \
    --config src/apps/exchange_market_data_playback/config_udp_slow.json \
    src/apps/exchange_market_data_generator/output.mdp &
MD_PID=$!
sleep 2

echo ""
echo "[3] Starting Algorithm in FOREGROUND (you'll see all output)..."
echo "    This will run for 10 seconds..."
echo ""
src/apps/client_algorithm/build/algo_template 239.255.0.1 12345 127.0.0.1 9000 10

echo ""
echo "Algorithm finished. Cleaning up..."
kill $ME_PID $MD_PID 2>/dev/null || true

echo "Done!"
