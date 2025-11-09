#!/bin/bash
# TIP: This script copies the debug binary to bin/debug after building.
# This makes orchestration, packaging, and CI/CD easier, while keeping the binary available in the build directory for local testing and debugging.
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_generator
# Purpose:      Debug build script for the market data generator
# Author:       Bryan Camp
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-debug"
BIN_DIR="${SCRIPT_DIR}/bin/debug"

echo "[exchange_market_data_generator] Cleaning debug build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[exchange_market_data_generator] Configuring CMake (Debug)..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -Wpedantic -O0"

echo "[exchange_market_data_generator] Building (Debug)..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Copy debug binary to bin/debug
if [ -f "exchange_market_data_generator" ]; then
    mkdir -p "$BIN_DIR"
    cp "exchange_market_data_generator" "$BIN_DIR/"
    echo "[exchange_market_data_generator] Debug binary copied to $BIN_DIR/exchange_market_data_generator"
else
    echo "[exchange_market_data_generator] Debug binary not found!"
fi

echo "[exchange_market_data_generator] Debug build complete!"
