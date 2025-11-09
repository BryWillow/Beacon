#!/bin/bash

BUILD_DIR="build-release"
BIN_DIR="../../../bin/release"

echo "Cleaning release build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "Configuring CMake (Release)..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -Wall -Wextra -Wpedantic" || exit 1

echo "Building (Release)..."
cmake --build . || exit 1

# Copy all expected binaries to bin/release
for exe in client_algorithm client_algo client_algo_hft algo_twap algo_template example_latency_tracking test_pillar test_cme; do
    if [ -f "$exe" ]; then
        mkdir -p "$BIN_DIR"
        cp "$exe" "$BIN_DIR/"
        echo "[$exe] Release binary copied to $BIN_DIR/$exe"
    fi
done

echo "Release build complete!"
