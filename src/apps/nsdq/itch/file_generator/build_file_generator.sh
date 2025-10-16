#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   Beacon
# @component Beacon Build System [file_generator]
# @file      build_file_generator
# @brief     Builds file_generator in debug|release configurations.
#            debug    - Includes ASan and cppcheck.
#            release  - Fully optimized, including O3 and LTO.
#            Binaries placed in Beacon/bin/file_generator/debug|release.
#            Build mode is determined by the user.
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# ANSI colors
BOLD="\033[1m"
YELLOW="\033[33m"
RED="\033[31m"
RESET="\033[0m"

# Directories
REPO_ROOT="$(git rev-parse --show-toplevel)"
BUILD_DIR="${REPO_ROOT}/build/file_generator"
BIN_DIR="${REPO_ROOT}/bin/file_generator"

# Default to debug if no argument
ACTION="${1:-debug}"

print_info() {
    echo ""
    echo "Build modes:"
    echo "  debug    - Includes ASan and cppcheck."
    echo "  release  - Fully optimized, including O3 and LTO."
    echo "  all      - Builds both debug and release."
    echo ""
    echo "Output:"
    echo "  debug    -> Binaries placed in Beacon/bin/file_generator/debug."
    echo "  release  -> Binaries placed in Beacon/bin/file_generator/release."
    echo ""
}

print_default_note() {
    echo -e "${BOLD}${YELLOW}No mode [debug|release|all|clean] specified. DEFAULTING TO 'debug'${RESET}"
    print_info
}

unknown_option() {
    echo -e "${BOLD}${RED}ERROR: Unknown build mode specified: $ACTION${RESET}"
    print_info
    exit 1
}

build_config() {
    local config="$1"
    local cmake_type=""
    local build_subdir="${BUILD_DIR}/${config}"
    local bin_subdir="${BIN_DIR}/${config}"

    # Map lowercase config to CMake's expected capitalization
    if [ "$config" == "debug" ]; then
        cmake_type="Debug"
    else
        cmake_type="Release"
    fi

    echo "-----------------------------------------"
    echo -e "${BOLD}Building file_generator ($config)${RESET}..."
    echo "Cleaning previous build..."
    rm -rf "$build_subdir" "$bin_subdir"

    mkdir -p "$build_subdir"
    mkdir -p "$bin_subdir"

    cmake -S "${REPO_ROOT}/src/apps/nsdq/itch/file_generator" \
          -B "$build_subdir" \
          -DCMAKE_BUILD_TYPE="$cmake_type" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$bin_subdir"

    cmake --build "$build_subdir" -- -j$(sysctl -n hw.ncpu)
    echo -e "${BOLD}Built file_generator ($config) -> $bin_subdir${RESET}"
}

clean_all() {
    echo "Cleaning all build and bin directories..."
    rm -rf "${BUILD_DIR}" "${BIN_DIR}"
    echo "Clean complete."
}

# Handle no argument
if [ -z "$1" ]; then
    print_default_note
fi

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
