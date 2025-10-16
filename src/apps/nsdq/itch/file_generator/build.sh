#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   Beacon
# @component Beacon Build System [file_generator]
# @file      build.sh
# @brief     Builds debug|release|all|clean versions of the file_generator.
#            Debug builds include sanitizers, cppcheck, and debug symbols.
#            Release builds are fully optimized.
# @author    Bryan Camp
# ---------------------------------------------------------------------------

REPO_ROOT="$(git rev-parse --show-toplevel)"
BUILD_DIR="${REPO_ROOT}/build/file_generator"
BIN_DIR="${REPO_ROOT}/bin/file_generator"

RED="\033[0;31m"
NC="\033[0m"

print_usage() {
    echo "Usage:"
    echo "  ./build.sh [debug|release|all|clean]"
    echo
    echo "Options:"
    echo "  debug     Build Debug configuration (sanitizers, cppcheck, debug symbols)"
    echo "  release   Build Release configuration (fully optimized)"
    echo "  all       Build both Debug and Release"
    echo "  clean     Remove build and bin directories"
}

build_config() {
    local config=$1
    local cmake_type=""
    local build_subdir="${BUILD_DIR}/${config}"
    local bin_subdir="${BIN_DIR}/${config}"

    # Map lowercase config to CMake's expected capitalization
    if [ "$config" == "debug" ]; then
        cmake_type="Debug"
    else
        cmake_type="Release"
    fi

    echo "Cleaning previous ${config} build..."
    rm -rf "${build_subdir}" "${bin_subdir}"

    echo "Building ${config}..."
    mkdir -p "${build_subdir}"
    mkdir -p "${bin_subdir}"

    echo "Repo root: ${REPO_ROOT}"
    echo "Build dir: ${build_subdir}"
    echo "Binary dir: ${bin_subdir}"

    cmake -S "${REPO_ROOT}/src/apps/nsdq/itch/file_generator" \
          -B "${build_subdir}" \
          -DCMAKE_BUILD_TYPE="${cmake_type}" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${bin_subdir}" \
          -DREPO_ROOT="${REPO_ROOT}"  # Pass repo root to CMake

    echo "Configured successfully. Starting build..."
    cmake --build "${build_subdir}" --config "${cmake_type}"
    echo "${config} build completed."
}

if [ $# -lt 1 ]; then
    echo -e "${RED}ERROR: missing build option.${NC}"
    print_usage
    exit 1
fi

case "$1" in
    debug)
        build_config debug
        ;;
    release)
        build_config release
        ;;
    all)
        build_config debug
        build_config release
        ;;
    clean)
        echo "Cleaning all build and bin directories..."
        rm -rf "${BUILD_DIR}" "${BIN_DIR}"
        echo "Clean completed."
        ;;
    *)
        echo -e "${RED}ERROR: unknown option: $1${NC}"
        echo
        print_usage
        exit 1
        ;;
esac
