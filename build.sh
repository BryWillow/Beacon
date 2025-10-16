#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   Beacon
# @component Beacon Build System [Pipeline]
# @file      build.sh
# @brief     Builds file_generator in debug|release configurations.
#            debug   - Includes ASan and cppcheck.
#            release - Fully optimized, including O3 and LTO.
#            Binaries placed in Beacon/bin/file_generator/debug|release.
#            Build mode is determined by the pipeline.
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# ANSI codes for bold colors
BOLD="\033[1m"
YELLOW="\033[33m"
RED="\033[31m"
RESET="\033[0m"

usage() {
    echo "Usage: $0 [debug|release|all|clean]"
    exit 1
}

# Default action
ACTION="${1:-debug}"   # default to debug if no argument provided

# Print note if no parameter given
if [ -z "$1" ]; then
    echo -e "No mode [debug|release|all|clean] specified. ${BOLD}${YELLOW}DEFAULTING TO 'debug'${RESET}"
    echo ""
    echo "Build modes:"
    echo "  debug    - Includes ASan and cppcheck."
    echo "  release  - Fully optimized, including O3 and LTO."
    echo "  all      - Builds both debug and release modes."
    echo ""

    echo "Output:"
    echo "  debug    -> Binaries placed in Beacon/bin/file_generator/debug."
    echo "  release  -> Binaries placed in Beacon/bin/file_generator/release."
    echo ""
fi

# Determine repo root
REPO_ROOT="$(git rev-parse --show-toplevel)"

# Build function
build_config() {
    local config="$1"
    echo "-----------------------------------------"
    echo "Building file_generator ($config)..."

    local build_dir="${REPO_ROOT}/build/file_generator/${config}"
    local bin_dir="${REPO_ROOT}/bin/file_generator/${config}"  # lowercase

    # Clean previous build
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    mkdir -p "$bin_dir"

    # Configure CMake
    cmake -S "${REPO_ROOT}/src/apps/nsdq/itch/file_generator" \
          -B "$build_dir" \
          -DCMAKE_BUILD_TYPE="$config" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$bin_dir"

    # Build
    cmake --build "$build_dir" -- -j$(sysctl -n hw.ncpu)
    echo "Built file_generator ($config) -> $bin_dir"
}

# Clean function
clean_all() {
    echo "Cleaning all builds..."
    rm -rf "${REPO_ROOT}/build/file_generator"
    rm -rf "${REPO_ROOT}/bin/file_generator"
    echo "Clean complete."
}

# Handle unknown options
unknown_option() {
    echo -e "${BOLD}${RED}ERROR: Unknown build mode specified${RESET}"
    echo ""
    echo "Build modes:"
    echo "  debug    - Includes ASan and cppcheck."
    echo "  release  - Fully optimized, including O3 and LTO."
    echo "  all      - Builds both debug and release modes."
    echo ""
    echo "Output:"
    echo "  debug    -> Binaries placed in Beacon/bin/file_generator/debug."
    echo "  release  -> Binaries placed in Beacon/bin/file_generator/release."
    echo ""
    exit 1
}

# Main case block
case "$ACTION" in
    debug)
        build_config "debug"
        ;;
    release)
        build_config "release"
        ;;
    all)
        build_config "debug"
        build_config "release"
        ;;
    clean)
        clean_all
        ;;
    *)
        unknown_option
        ;;
esac
