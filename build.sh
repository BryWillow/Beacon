#!/bin/bash
# =============================================================================
# Project:      Beacon
# Purpose:      Top-level clean build script for all applications
# Author:       Bryan Camp
# =============================================================================

set -e

# Get the directory where this script is located (experimental root)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "=========================================="
echo "  Beacon - Building All Applications"
echo "=========================================="

echo ""
echo "[Beacon] Cleaning top-level build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[Beacon] Configuring CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "[Beacon] Building all targets..."
cmake --build . -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo ""
echo "=========================================="
echo "  Build Complete!"
echo "=========================================="
echo ""
echo "Binaries:"
echo "  - exchange_market_data_generator: ${BUILD_DIR}/src/apps/exchange_market_data_generator/exchange_market_data_generator"
echo "  - exchange_market_data_playback:  ${BUILD_DIR}/src/apps/exchange_market_data_playback/exchange_market_data_playback"
echo ""
