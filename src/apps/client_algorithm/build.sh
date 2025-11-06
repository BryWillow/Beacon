#!/bin/bash

APP_NAME="client_algorithm"
BUILD_DIR="build"

echo "[$APP_NAME] Cleaning build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "[$APP_NAME] Configuring CMake..."
cd "$BUILD_DIR"
cmake .. || { echo "CMake configuration failed"; exit 1; }

echo "[$APP_NAME] Building..."
cmake --build . || { echo "Build failed"; exit 1; }

echo "[$APP_NAME] Build complete!"
echo "[$APP_NAME] Binary location: $(pwd)/$APP_NAME"
