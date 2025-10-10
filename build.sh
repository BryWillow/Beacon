#!/bin/bash
###############################################################################
# @file        build.sh
# @author      Bryan Camp
# @component   Beacon Build System
# @description Fully bulletproof build script for Beacon Market Data Pipeline
###############################################################################

set -euo pipefail

# ------------------------------ #
# Colors
# ------------------------------ #
RED=$'\e[31m'
GREEN=$'\e[32m'
BLUE=$'\e[34m'
YELLOW=$'\e[33m'
RESET=$'\e[0m'

# ------------------------------ #
# Configuration
# ------------------------------ #
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="$PROJECT_ROOT/bin"
BUILD_DIR="$PROJECT_ROOT/build"
VERSION_FILE="$PROJECT_ROOT/VERSION"
MAX_WARNINGS=5
MAX_DESC_LEN=80

# Default flags
SYMBOLS=true
SANITIZE=true
RUN_TESTS=true
SKIP_TESTS=false
CREATE_COMMIT_MSG=false
VERSION_OVERRIDE=""

# ------------------------------ #
# Usage
# ------------------------------ #
print_usage() {
cat <<EOL
Usage: $0 [debug|release|all|clean] [options]

Modes (required):
  debug   Build Debug with symbols and sanitizers
  release Build Release (sanitizers ignored)
  all     Build Debug then Release
  clean   Remove all build artifacts

Available flags (optional):
  -s:true|false    Enable sanitizers (Debug only, default: true)
  -t:true|false    Run unit tests (default: true)
  -n:true|false    Skip tests (default: false)
  -c:true|false    Suggest commit message based on changes
  -v:<ver>        Override version MAJOR.MINOR.PATCH (BUILD always increments)

Examples:
  ./build.sh debug
  ./build.sh release -t:false
  ./build.sh all
EOL
}

# ------------------------------ #
# Parse arguments
# ------------------------------ #
if [ $# -lt 1 ]; then
    echo -e "${RED}ERROR: No build mode specified${RESET}"
    print_usage
    exit 1
fi

MODE="$1"
shift

while [[ $# -gt 0 ]]; do
    case "$1" in
        -s:true) SANITIZE=true; shift ;;
        -s:false) SANITIZE=false; shift ;;
        -t:true) RUN_TESTS=true; shift ;;
        -t:false) RUN_TESTS=false; shift ;;
        -n:true) SKIP_TESTS=true; shift ;;
        -n:false) SKIP_TESTS=false; shift ;;
        -c:true) CREATE_COMMIT_MSG=true; shift ;;
        -c:false) CREATE_COMMIT_MSG=false; shift ;;
        -v:*) VERSION_OVERRIDE="${1#*:}"; shift ;;
        *) echo -e "${RED}ERROR: Unknown option: $1${RESET}"; print_usage; exit 1 ;;
    esac
done

# ------------------------------ #
# Version increment
# ------------------------------ #
if [ ! -f "$VERSION_FILE" ]; then
    echo "1.0.0.0" > "$VERSION_FILE"
fi

VERSION=$(cat "$VERSION_FILE")
MAJOR=$(echo "$VERSION" | cut -d. -f1)
MINOR=$(echo "$VERSION" | cut -d. -f2)
PATCH=$(echo "$VERSION" | cut -d. -f3)
BUILD_NUM=$(echo "$VERSION" | cut -d. -f4)

if [[ -n "$VERSION_OVERRIDE" ]]; then
    MAJOR=$(echo "$VERSION_OVERRIDE" | cut -d. -f1)
    MINOR=$(echo "$VERSION_OVERRIDE" | cut -d. -f2)
    PATCH=$(echo "$VERSION_OVERRIDE" | cut -d. -f3)
fi

BUILD_NUM=$((BUILD_NUM+1))
VERSION="${MAJOR}.${MINOR}.${PATCH}.${BUILD_NUM}"
echo "$VERSION" > "$VERSION_FILE"

# ------------------------------ #
# Helpers
# ------------------------------ #
clean_all() {
    rm -rf "$BUILD_DIR" "$BIN_DIR"
    rm -f "$PROJECT_ROOT/CMakeCache.txt" "$PROJECT_ROOT/Makefile" \
          "$PROJECT_ROOT/cmake_install.cmake" "$PROJECT_ROOT/compile_commands.json"
    echo "Clean completed."
}

create_stubs() {
    EXEC_DIR="$PROJECT_ROOT/src/apps/nsdq/execution"
    MD_DIR="$PROJECT_ROOT/src/apps/nsdq/market_data"
    ME_DIR="$PROJECT_ROOT/src/apps/nsdq/matching_engine"
    mkdir -p "$EXEC_DIR" "$MD_DIR" "$ME_DIR"

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

    [[ -f "$ME_DIR/main.cpp" ]] || cat <<EOL > "$ME_DIR/main.cpp"
#include <iostream>
int main() { std::cout << "NSDQ Matching Engine running" << std::endl; return 0; }
EOL
}

build_type() {
    local BUILD_TYPE="$1"
    local BUILD_OUT="$BIN_DIR/$BUILD_TYPE"
    local BUILD_DIR_LOCAL="$BUILD_DIR/$BUILD_TYPE"

    mkdir -p "$BUILD_OUT" "$BUILD_DIR_LOCAL"
    cd "$BUILD_DIR_LOCAL"

    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BUILD_OUT" \
          "$PROJECT_ROOT"

    cmake --build . --config "$BUILD_TYPE"
    cd "$PROJECT_ROOT"
}

# ------------------------------ #
# Run unit tests (safe if none exist)
# ------------------------------ #
run_unit_tests() {
    [[ "$SKIP_TESTS" == true ]] && echo "none" && return

    shopt -s nullglob
    TEST_BINS=( "$BIN_DIR/debug/test_*" )
    shopt -u nullglob

    if [[ ${#TEST_BINS[@]} -eq 0 ]]; then
        echo "none"
        return
    fi

    for TEST_BIN in "${TEST_BINS[@]}"; do
        if ! "$TEST_BIN"; then
            echo "failed"
            return
        fi
    done

    echo "all_passed"
}

truncate_text() {
    local text="$1"
    local maxlen="$2"
    if [[ ${#text} -le $maxlen ]]; then
        echo "$text"
    else
        echo "${text:0:$maxlen}..."
    fi
}

write_release_notes() {
    local BUILD_TYPE="$1"
    local TEST_RESULTS_RAW="$2"
    local OUT="$BIN_DIR/$BUILD_TYPE/release_notes.txt"
    mkdir -p "$(dirname "$OUT")"

    BREAKING_DESC="[None]"
    MINOR_DESC="[None]"
    PATCH_DESC="[None]"

    if git rev-parse --git-dir > /dev/null 2>&1; then
        COMMITS=()
        while IFS= read -r line; do
            COMMITS+=("$line")
        done < <(git log -3 --pretty=format:'%s')

        for commit in "${COMMITS[@]}"; do
            if [[ $commit == *"[BREAKING]"* ]]; then
                BREAKING_DESC=$(truncate_text "${commit//\[BREAKING\]/}" $MAX_DESC_LEN)
                MAJOR=$((MAJOR>0 ? MAJOR : 1))
            elif [[ $commit == *"[MINOR]"* ]]; then
                MINOR_DESC=$(truncate_text "${commit//\[MINOR\]/}" $MAX_DESC_LEN)
                MINOR=$((MINOR>0 ? MINOR : 1))
            elif [[ $commit == *"[PATCH]"* ]]; then
                PATCH_DESC=$(truncate_text "${commit//\[PATCH\]/}" $MAX_DESC_LEN)
                PATCH=$((PATCH>0 ? PATCH : PATCH))
            fi
        done
    fi

    UPDATES=()
    if git rev-parse --git-dir > /dev/null 2>&1; then
        while IFS= read -r line; do
            UPDATES+=("- $line")
        done < <(git log -3 --pretty=format:'%s')
    else
        UPDATES=("- None")
    fi

    WARN_FILE="$BUILD_DIR/$BUILD_TYPE/compile_warnings.txt"
    if [[ -f "$WARN_FILE" ]]; then
        TOTAL_WARNINGS=$(wc -l < "$WARN_FILE")
    else
        TOTAL_WARNINGS=0
    fi

    WARNINGS=""
    if [[ $TOTAL_WARNINGS -gt 0 ]]; then
        WARNINGS=$(head -n 3 "$WARN_FILE" 2>/dev/null || echo "")
    fi

    WARN_LINE="Build Warning(s)  : ${TOTAL_WARNINGS} warning$( [[ $TOTAL_WARNINGS -ne 1 ]] && echo s )"

    case "$TEST_RESULTS_RAW" in
        none) TEST_LINE_FILE="None Found" ;;
        all_passed) TEST_LINE_FILE="All Passed [0/0]" ;;
        failed) TEST_LINE_FILE="Some Tests Failed" ;;
        *) TEST_LINE_FILE="$TEST_RESULTS_RAW" ;;
    esac

    {
        echo "Version           : $VERSION"
        echo ""
        echo "Build Date        : $(date '+%Y-%m-%d %H:%M:%S')"
        echo "Built By          : $(whoami)"
        echo "Build Mode        : $BUILD_TYPE"
        echo "Command-Line      : ./build.sh $MODE"
        echo "Breaking Changes  : [None]"
        echo "$WARN_LINE"
        echo "Build Output      : Beacon/bin/$BUILD_TYPE"
        echo "Updates:"
        for u in "${UPDATES[@]}"; do
            echo "  $u"
        done
        echo "Test Results      : $TEST_LINE_FILE"
        echo ""
        echo "Version Details:"
        printf "  %-5s: %-3s [Breaking Change(s): %s]\n" "Major" "$MAJOR" "$BREAKING_DESC"
        printf "  %-5s: %-3s [Minor Change(s)   : %s]\n" "Minor" "$MINOR" "$MINOR_DESC"
        printf "  %-5s: %-3s [Patch Change(s)   : %s]\n" "Patch" "$PATCH" "$PATCH_DESC"
        printf "  %-5s: %-3s\n" "Build" "$BUILD_NUM"
    } > "$OUT"
}

# ------------------------------ #
# Main
# ------------------------------ #
create_stubs

case "$MODE" in
    clean)
        clean_all
        ;;
    debug)
        clean_all
        build_type "Debug"
        TEST_RESULT=$(run_unit_tests)
        write_release_notes "debug" "$TEST_RESULT"
        ;;
    release)
        clean_all
        build_type "Release"
        TEST_RESULT=$(run_unit_tests)
        write_release_notes "release" "$TEST_RESULT"
        ;;
    all)
        clean_all
        build_type "Debug"
        TEST_RESULT=$(run_unit_tests)
        write_release_notes "debug" "$TEST_RESULT"
        build_type "Release"
        TEST_RESULT=$(run_unit_tests)
        write_release_notes "release" "$TEST_RESULT"
        ;;
    *)
        echo -e "${RED}ERROR: Unknown mode: $MODE${RESET}"
        print_usage
        exit 1
        ;;
esac
