            #!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="exchange_matching_engine"
BUILD_DIR="${SCRIPT_DIR}/build-debug"
BIN_DIR="${SCRIPT_DIR}/bin/debug"

echo "[$APP_NAME] Cleaning debug build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

echo "[$APP_NAME] Configuring CMake (Debug)..."
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -Wall -Wextra -Wpedantic -O0" || exit 1

echo "[$APP_NAME] Building (Debug)..."
cmake --build . || exit 1

# Move binary to bin/debug
if [ -f "exchange_matching_engine" ]; then
    mkdir -p "$BIN_DIR"
    cp "exchange_matching_engine" "$BIN_DIR/"
    echo "[$APP_NAME] Debug binary copied to $BIN_DIR/exchange_matching_engine"
else
    echo "[$APP_NAME] Debug binary not found!"
fi

echo "[$APP_NAME] Debug build complete!"
