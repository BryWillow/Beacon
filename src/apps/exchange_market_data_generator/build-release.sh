#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_generator
# Purpose:      Release build script for the market data generator
# Author:       Bryan Camp
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-release"
BIN_DIR="../../../bin/release"

echo "[exchange_market_data_generator] Cleaning release build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[exchange_market_data_generator] Configuring CMake (Release)..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -Wall -Wextra -Wpedantic"

echo "[exchange_market_data_generator] Building (Release)..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Move binary to bin/release
if [ -f "exchange_market_data_generator" ]; then
    mkdir -p "$BIN_DIR"
    cp "exchange_market_data_generator" "$BIN_DIR/"
    echo "[exchange_market_data_generator] Release binary moved to $BIN_DIR/exchange_market_data_generator"
else
    echo "[exchange_market_data_generator] Release binary not found!"
fi

echo "[exchange_market_data_generator] Release build complete!"
