#!/usr/bin/env bash
# Abstract build script for Beacon
# Delegates to app-specific build scripts in the `pipeline` directory.

set -euo pipefail

# ANSI codes for bold colors
BOLD="\033[1m"
YELLOW="\033[33m"
RED="\033[31m"
RESET="\033[0m"

# Function to display usage
usage() {
    echo "Usage: $0 [action]"
    echo "Actions: debug, release, all, clean"
    echo "Binaries are located in:"
    echo "  debug    -> bin/<app_name>/debug"
    echo "  release  -> bin/<app_name>/release"
    exit 1
}

# Build function
build_config() {
    local app_name="$1"
    local config="$2"
    echo "-----------------------------------------"
    echo "Building $app_name ($config)..."

    local build_dir="${REPO_ROOT}/build/${app_name}/${config}"
    local bin_dir="${REPO_ROOT}/bin/${app_name}/${config}"  # lowercase

    # Clean previous build
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    mkdir -p "$bin_dir"

    # Configure CMake
    cmake -S "${REPO_ROOT}/src/apps/nsdq/itch/${app_name}" \
          -B "$build_dir" \
          -DCMAKE_BUILD_TYPE="$config" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$bin_dir"

    # Build
    cmake --build "$build_dir" -- -j$(sysctl -n hw.ncpu)
    echo "Built $app_name ($config) -> $bin_dir"
}

# Clean function
clean_all() {
    local app_name="$1"
    echo "Cleaning all builds for $app_name..."
    rm -rf "${REPO_ROOT}/build/${app_name}"
    rm -rf "${REPO_ROOT}/bin/${app_name}"
    echo "Clean complete."
}

# Handle unknown options
unknown_option() {
    echo -e "${BOLD}${RED}ERROR: Unknown build mode specified${RESET}"
    usage
}

# Ensure action is provided
if [ "$#" -lt 1 ]; then
    usage
fi

ACTION="$1"
REPO_ROOT="$(git rev-parse --show-toplevel)"
APPS=("file_generator" "udp_replayer" "matching_engine")

# Main case block
case "$ACTION" in
    debug|release|all|clean)
        for app in "${APPS[@]}"; do
            case "$ACTION" in
                debug|release)
                    "${REPO_ROOT}/pipeline/build/build_${app}.sh" "$ACTION"
                    ;;
                all)
                    "${REPO_ROOT}/pipeline/build/build_${app}.sh" "debug"
                    "${REPO_ROOT}/pipeline/build/build_${app}.sh" "release"
                    ;;
                clean)
                    "${REPO_ROOT}/pipeline/build/build_${app}.sh" "clean"
                    ;;
            esac
        done
        ;;
    *)
        unknown_option
        ;;
esac