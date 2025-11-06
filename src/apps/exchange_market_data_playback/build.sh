#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_playback
# Purpose:      Clean build script for the market data playback
# Author:       Bryan Camp
# =============================================================================

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "[exchange_market_data_playback] Cleaning build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[exchange_market_data_playback] Configuring CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "[exchange_market_data_playback] Building..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo "[exchange_market_data_playback] Build complete!"
echo "[exchange_market_data_playback] Binary location: ${BUILD_DIR}/exchange_market_data_playback"
