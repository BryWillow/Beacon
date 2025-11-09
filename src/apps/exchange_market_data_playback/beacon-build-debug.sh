#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_playback
# Purpose:      Debug build script for the market data playback
# Author:       Bryan Camp
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-debug"
BIN_DIR="../../../bin/debug"

echo "[exchange_market_data_playback] Cleaning debug build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[exchange_market_data_playback] Configuring CMake (Debug)..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -Wpedantic -O0"

echo "[exchange_market_data_playback] Building (Debug)..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Move binary to bin/debug
if [ -f "exchange_market_data_playback" ]; then
    mkdir -p "$BIN_DIR"
    cp "exchange_market_data_playback" "$BIN_DIR/"
    echo "[exchange_market_data_playback] Debug binary moved to $BIN_DIR/exchange_market_data_playback"
else
    echo "[exchange_market_data_playback] Debug binary not found!"
fi

echo "[exchange_market_data_playback] Debug build complete!"
