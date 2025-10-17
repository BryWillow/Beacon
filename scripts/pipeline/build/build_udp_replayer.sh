#!/usr/bin/env bash
# Build script for udp_replayer app

set -euo pipefail

echo "Building udp_replayer app..."

REPO_ROOT="$(git rev-parse --show-toplevel)"
BUILD_DIR="$REPO_ROOT/build/udp_replayer/$1"
BIN_DIR="$REPO_ROOT/bin/udp_replayer/$1"

mkdir -p "$BUILD_DIR" "$BIN_DIR"

cmake -S "$REPO_ROOT/src/apps/udp_replayer" \
      -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE="$1" \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BIN_DIR"

cmake --build "$BUILD_DIR" -- -j$(sysctl -n hw.ncpu)
echo "Built udp_replayer ($1) -> $BIN_DIR"
