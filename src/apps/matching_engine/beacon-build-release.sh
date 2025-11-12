#!/bin/bash

APP_NAME="exchange_matching_engine"
BUILD_DIR="build-release"
BIN_DIR="../../../bin/release"

echo "[$APP_NAME] Cleaning release build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[$APP_NAME] Configuring CMake (Release)..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native -Wall -Wextra -Wpedantic" || { echo "CMake configuration failed"; exit 1; }

echo "[$APP_NAME] Building (Release)..."
cmake --build . || { echo "Build failed"; exit 1; }

# Move binary to bin/release
if [ -f "$APP_NAME" ]; then
    mkdir -p "$BIN_DIR"
    cp "$APP_NAME" "$BIN_DIR/"
    echo "[$APP_NAME] Release binary moved to $BIN_DIR/$APP_NAME"
else
    echo "[$APP_NAME] Release binary not found!"
fi

echo "[$APP_NAME] Release build complete!"
