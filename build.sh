#!/usr/bin/env bash
set -euo pipefail

# --------------------------------------------------------------------
# build.sh
# One-shot build script for Beacon
# Usage: ./build.sh [Debug|Release|All|Clean]
# --------------------------------------------------------------------

# -------------------------------
# Validate input
# -------------------------------
if [ $# -ne 1 ]; then
    echo "Usage: $0 [Debug|Release|All|Clean]"
    exit 1
fi

CMD="$1"
PROJECT_ROOT="$(pwd)"
BIN_DIR="$PROJECT_ROOT/bin"
BUILD_DIR="$PROJECT_ROOT/build_temp"
NUM_COMMITS=20

# -------------------------------
# Clean function
# -------------------------------
clean() {
    echo "[Clean] Removing build artifacts and temporary directories..."
    rm -rf "$BUILD_DIR"
    rm -rf "$BIN_DIR"
    echo "[Clean] Done."
}

# -------------------------------
# Handle clean command immediately
# -------------------------------
if [ "$CMD" == "Clean" ]; then
    clean
    exit 0
fi

# -------------------------------
# Prepare directories
# -------------------------------
echo "[Build] Preparing directories..."
rm -rf "$BUILD_DIR"
mkdir -p "$BIN_DIR/Debug"
mkdir -p "$BIN_DIR/Release"

# -------------------------------
# Helper function: CMake build
# -------------------------------
cmake_build() {
    local type="$1"
    local out_dir="$BIN_DIR/$type"

    echo "[Build] Configuring $type build..."
    cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
          -DCMAKE_BUILD_TYPE="$type" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$out_dir" \
          $( [ "$type" == "Release" ] && echo "-DCMAKE_CXX_FLAGS_RELEASE='-O3 -DNDEBUG -flto' -DCMAKE_EXE_LINKER_FLAGS_RELEASE='-flto'" )

    echo "[Build] Building $type..."
    cmake --build "$BUILD_DIR" --config "$type" -j$(nproc)

    echo "[Build] Generating release notes for $type..."
    git log -n $NUM_COMMITS --pretty=format:"%h %ad %an %s" --date=short > "$out_dir/release_notes.txt"

    if [ "$type" == "Debug" ]; then
        echo "[Build] Running tests..."
        ctest --test-dir "$BUILD_DIR" --output-on-failure
    fi

    if [ "$type" == "Release" ]; then
        echo "[Build] Stripping binaries for production..."
        find "$out_dir" -type f -perm +111 -exec strip {} \; || true
    fi
}

# -------------------------------
# Execute builds
# -------------------------------
case "$CMD" in
    Debug)
        cmake_build Debug
        ;;
    Release)
        cmake_build Release
        ;;
    All)
        cmake_build Debug
        cmake_build Release
        ;;
    *)
        echo "Invalid command: $CMD"
        echo "Usage: $0 [Debug|Release|All|Clean]"
        exit 1
        ;;
esac

# -------------------------------
# Cleanup temporary build directory
# -------------------------------
echo "[Build] Cleaning temporary build directory..."
rm -rf "$BUILD_DIR"

echo "[Build] Done!"
