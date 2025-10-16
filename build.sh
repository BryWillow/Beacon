#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# build.sh - Build Beacon apps
# ---------------------------------------------------------------------------

set -e

BUILD_TYPE=${1:-Debug}  # Default to Debug if no argument
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build_$BUILD_TYPE"

echo "=============================="
echo " Building Beacon ($BUILD_TYPE) "
echo "=============================="

# Optional clean
if [[ "$2" == "clean" ]]; then
    echo "Cleaning $BUILD_DIR..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure CMake
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build all targets
make -j

echo "=============================="
echo " Build complete ($BUILD_TYPE) "
echo "=============================="
