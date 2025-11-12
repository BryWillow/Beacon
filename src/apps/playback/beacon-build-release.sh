#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_playback
# Purpose:      Release build script for the market data playback
# Author:       Bryan Camp
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-release"
BIN_DIR="../../../bin/release"

echo "[exchange_market_data_playback] Cleaning release build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[exchange_market_data_playback] Configuring CMake (Release)..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -Wall -Wextra -Wpedantic"

echo "[exchange_market_data_playback] Building (Release)..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Move binary to bin/release
if [ -f "exchange_market_data_playback" ]; then
    mkdir -p "$BIN_DIR"
    cp "exchange_market_data_playback" "$BIN_DIR/"
    echo "[exchange_market_data_playback] Release binary moved to $BIN_DIR/exchange_market_data_playback"
else
    echo "[exchange_market_data_playback] Release binary not found!"
fi

echo "[exchange_market_data_playback] Release build complete!"
