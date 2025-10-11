#!/bin/bash
###############################################################################
# @file        run_tests.sh
# @project     Beacon
# @component   Unit Test Runner
# @description Cleans previous test builds and runs all tests
# @author      Bryan Camp
###############################################################################

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_tests"

echo "Cleaning previous test build ar#!/bin/bash
# ---------------------------------------------------------------------------
# @file        run_tests.sh
# @description Build and run all unit tests (self-contained)
# ---------------------------------------------------------------------------

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_tests"

echo "Cleaning previous test build artifacts..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Configuring CMake..."
cmake -S "$SCRIPT_DIR/tests" -B "$BUILD_DIR" \
      -DGOOGLETEST_ROOT="$SCRIPT_DIR/vendor/googletest" \
      -DCMAKE_CXX_STANDARD=20

echo "Building all tests..."
cmake --build "$BUILD_DIR" --parallel

echo "Running all tests..."
cd "$BUILD_DIR"
ctest --output-on-failure

echo "All tests completed."
tifacts..."
rm -rf "$BUILD_DIR"

echo "Building all tests..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure tests project
cmake -S "$SCRIPT_DIR/tests" -B "$BUILD_DIR"

# Build all tests
cmake --build "$BUILD_DIR" -- -j$(sysctl -n hw.ncpu)

# Run all tests
echo "Running all tests..."
ctest --output-on-failure

echo "All tests completed."
