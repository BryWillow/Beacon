#!/bin/bash

# TIP: This script copies all expected binaries to bin/debug after building.
# This makes orchestration, packaging, and CI/CD easier, while keeping binaries available in the build directory for local testing and debugging.

BUILD_DIR="build-debug"
BIN_DIR="../../../bin/debug"

echo "Cleaning debug build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "Configuring CMake (Debug)..."
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -Wpedantic -O0" || exit 1

echo "Building (Debug)..."
cmake --build . || exit 1

# Copy all expected binaries to bin/debug
for exe in client_algorithm client_algo client_algo_hft algo_twap algo_template example_latency_tracking test_pillar test_cme; do
    if [ -f "$exe" ]; then
        mkdir -p "$BIN_DIR"
        cp "$exe" "$BIN_DIR/"
        echo "[$exe] Debug binary copied to $BIN_DIR/$exe"
    fi
done

echo "Debug build complete!"
