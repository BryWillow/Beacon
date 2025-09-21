#!/bin/bash
###############################################################################
# @file        build.sh
# @author      Bryan Camp
# @brief       Fully bulletproof build script for Market Data Pipeline
# @details     Handles Debug, Release, All, Clean builds with:
#              - Out-of-source builds (bin/Debug, bin/Release)
#              - Stub generation for missing main.cpp files
#              - Removal of old *_v1 binaries
#              - Release notes & rebuild scripts
#              - Optional Debug unit tests
# @usage       ./build.sh [Debug|Release|All|Clean] [--run-tests=true|false]
###############################################################################

set -e

#==============================#
# Command-line argument parsing
#==============================#
if [ -z "$1" ]; then
    echo "Usage: ./build.sh [Debug|Release|All|Clean] [--run-tests=true|false]"
    exit 1
fi

MODE=$1
RUN_TESTS=true
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
GIT_HASH=$(git rev-parse --short HEAD)
MAX_ARCHIVES=5

if [ ! -z "$2" ]; then
    if [ "$MODE" != "Debug" ]; then
        echo "Error: --run-tests option only valid for Debug builds."
        exit 1
    fi
    case "$2" in
        --run-tests=false) RUN_TESTS=false ;;
        --run-tests=true)  RUN_TESTS=true ;;
        *) echo "Unknown option: $2"; exit 1 ;;
    esac
fi

#==============================#
# Function: CLEAN_ALL
# @brief    Remove all build directories, binaries, and root-level CMake artifacts
#==============================#
CLEAN_ALL() {
    rm -rf "$PROJECT_ROOT/build"
    rm -rf "$PROJECT_ROOT/bin/Debug"
    rm -rf "$PROJECT_ROOT/bin/Release"
    rm -f "$PROJECT_ROOT/CMakeCache.txt"
    rm -rf "$PROJECT_ROOT/CMakeFiles"
    rm -f "$PROJECT_ROOT/Makefile"
    rm -f "$PROJECT_ROOT/cmake_install.cmake"
    rm -f "$PROJECT_ROOT/compile_commands.json"
}

#==============================#
# Function: CREATE_STUBS
# @brief    Generate stub main.cpp files if missing
#==============================#
CREATE_STUBS() {
    EXEC_DIR="$PROJECT_ROOT/src/apps/nsdq/execution"
    MD_DIR="$PROJECT_ROOT/src/apps/nsdq/market_data"
    mkdir -p "$EXEC_DIR" "$MD_DIR"

    [[ -f "$EXEC_DIR/main.cpp" ]] || cat <<EOL > "$EXEC_DIR/main.cpp"
#include <iostream>
int main() { std::cout << "NSDQ Execution running" << std::endl; return 0; }
EOL

    MD_FILES=("generator.cpp" "listener.cpp" "replayer.cpp")
    MD_NAMES=("Generator" "Listener" "Replayer")
    for i in "${!MD_FILES[@]}"; do
        file="${MD_FILES[$i]}"
        name="${MD_NAMES[$i]}"
        [[ -f "$MD_DIR/$file" ]] || cat <<EOL > "$MD_DIR/$file"
#include <iostream>
int main() { std::cout << "NSDQ Market Data $name running" << std::endl; return 0; }
EOL
    done
}

#==============================#
# Function: BUILD_TYPE
# @brief    Perform CMake build out-of-source
# @param    BUILD   Debug or Release
#==============================#
BUILD_TYPE() {
    BUILD=$1
    BUILD_DIR="$PROJECT_ROOT/build_$BUILD"
    BIN_DIR="$PROJECT_ROOT/bin/$BUILD"

    # Clean stale build artifacts
    rm -rf "$BUILD_DIR"
    rm -f  "$PROJECT_ROOT/CMakeCache.txt"
    rm -rf "$PROJECT_ROOT/CMakeFiles"
    rm -f  "$PROJECT_ROOT/Makefile"
    rm -f  "$PROJECT_ROOT/cmake_install.cmake"
    rm -f  "$PROJECT_ROOT/compile_commands.json"

    # Recreate build + bin directories
    mkdir -p "$BUILD_DIR" "$BIN_DIR"

    # Configure & build
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=$BUILD \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BIN_DIR" \
          "$PROJECT_ROOT"
    cmake --build . --config $BUILD
    cd "$PROJECT_ROOT"
}

#==============================#
# Function: RUN_UNIT_TESTS
# @brief    Execute unit tests (Debug only)
#==============================#
RUN_UNIT_TESTS() {
    [[ "$RUN_TESTS" == false ]] && echo "Tests skipped" && return
    TEST_BIN="$PROJECT_ROOT/bin/Debug/test_runner"
    if [[ -f "$TEST_BIN" ]]; then
        if "$TEST_BIN"; then
            echo "All tests passed"
        else
            echo "Some tests failed"
        fi
    else
        echo "No tests found"
    fi
}

#==============================#
# Function: WRITE_RELEASE_NOTES
# @brief    Generate release notes for current build
# @param    BUILD          Debug or Release
# @param    TEST_RESULTS   Results of unit tests
#==============================#
WRITE_RELEASE_NOTES() {
    BUILD=$1
    TEST_RESULTS=$2
    BIN_DIR="$PROJECT_ROOT/bin/$BUILD"
    mkdir -p "$BIN_DIR"

    {
        echo "Build type        : $BUILD"
        echo "Git commit        : $GIT_HASH"
        echo " "
        git log -20 --pretty=format:"%h %s"
        echo " "
        echo "Test results      : $TEST_RESULTS"
        echo "Rebuild script    : rebuild_${TIMESTAMP}_${GIT_HASH}.sh"
    } > "$BIN_DIR/release_notes.txt"
}

#==============================#
# Function: ARCHIVE_BUILD
# @brief    Archive historical rebuild script and release notes
# @param    BUILD   Debug or Release
#==============================#
ARCHIVE_BUILD() {
    BUILD=$1
    BIN_DIR="$PROJECT_ROOT/bin/$BUILD"
    ARCHIVE_DIR="$BIN_DIR/Archive"
    mkdir -p "$ARCHIVE_DIR"

    REBUILD_SCRIPT="$ARCHIVE_DIR/rebuild_${TIMESTAMP}_${GIT_HASH}.sh"
    RELEASE_NOTES_ARCHIVE="$ARCHIVE_DIR/release_notes_${TIMESTAMP}_${GIT_HASH}.txt"

    cat <<EOL > "$REBUILD_SCRIPT"
#!/bin/bash
set -e
git checkout $GIT_HASH
mkdir -p build_$BUILD
cd build_$BUILD
cmake -DCMAKE_BUILD_TYPE=$BUILD ../..
cmake --build . --config $BUILD
cd ../..
EOL
    chmod +x "$REBUILD_SCRIPT"
    cp "$BIN_DIR/release_notes.txt" "$RELEASE_NOTES_ARCHIVE"

    COUNT=$(ls -1 "$ARCHIVE_DIR" | wc -l)
    if [ $COUNT -gt $((MAX_ARCHIVES*2)) ]; then
        ls -1t "$ARCHIVE_DIR" | tail -n +$((MAX_ARCHIVES*2+1)) | xargs -I {} rm "$ARCHIVE_DIR/{}"
    fi
}

#==============================#
# Function: CLEAN_OLD_V1_BINARIES
# @brief    Remove old _v1 binaries from bin directories
#==============================#
CLEAN_OLD_V1_BINARIES() {
    for BUILD in Debug Release; do
        BIN_DIR="$PROJECT_ROOT/bin/$BUILD"
        if [ -d "$BIN_DIR" ]; then
            find "$BIN_DIR" -maxdepth 1 -type f -name "*_v1" -exec rm -f {} \;
        fi
    done
}

#==============================#
# Main execution
#==============================#
CREATE_STUBS

case "$MODE" in
    Clean)
        CLEAN_ALL
        echo "Clean completed."
        ;;
    Debug)
        BUILD_TYPE "Debug"
        TEST_RESULTS=$(RUN_UNIT_TESTS || echo "Some tests failed")
        WRITE_RELEASE_NOTES "Debug" "$TEST_RESULTS"
        ARCHIVE_BUILD "Debug"
        CLEAN_OLD_V1_BINARIES
        echo "Debug build succeeded."
        ;;
    Release)
        BUILD_TYPE "Release"
        WRITE_RELEASE_NOTES "Release" "Not applicable"
        ARCHIVE_BUILD "Release"
        CLEAN_OLD_V1_BINARIES
        echo "Release build succeeded."
        ;;
    All)
        CLEAN_ALL
        BUILD_TYPE "Debug"
        TEST_RESULTS=$(RUN_UNIT_TESTS || echo "Some tests failed")
        WRITE_RELEASE_NOTES "Debug" "$TEST_RESULTS"
        ARCHIVE_BUILD "Debug"
        BUILD_TYPE "Release"
        WRITE_RELEASE_NOTES "Release" "Not applicable"
        ARCHIVE_BUILD "Release"
        CLEAN_OLD_V1_BINARIES
        echo "All builds succeeded."
        ;;
    *)
        echo "Unknown mode: $MODE"
        exit 1
        ;;
esac

