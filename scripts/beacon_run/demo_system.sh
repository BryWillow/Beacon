#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
APPS=(
    "exchange_matching_engine"
    "exchange_market_data_generator"
    "exchange_market_data_playback"
    "client_algorithm"
)

echo "═══════════════════════════════════════════════════════"
echo "        Beacon End-to-End Demo: Clean & Run            "
echo "═══════════════════════════════════════════════════════"

# Clean previous builds and logs
echo "[CLEAN] Removing previous build and log artifacts..."
rm -rf "$REPO_ROOT/logs" "$REPO_ROOT/scripts/logs"
for APP in "${APPS[@]}"; do
    rm -rf "$REPO_ROOT/src/apps/$APP/build-debug"
done
mkdir -p "$REPO_ROOT/logs"

# Build all apps (Debug)
echo "[BUILD] Building all apps (Debug)..."
for APP in "${APPS[@]}"; do
    bash "$REPO_ROOT/src/apps/$APP/beacon-build-debug.sh"
done

# Run each app in demo mode (if available)
echo "[RUN] Running demo for each app..."
for APP in "${APPS[@]}"; do
    BIN="$REPO_ROOT/src/apps/$APP/bin/debug/$APP"
    if [ -x "$BIN" ]; then
        echo "[DEMO] Running $APP..."
        "$BIN" --demo || echo "[WARN] $APP demo exited with non-zero status"
    else
        echo "[WARN] $APP binary not found: $BIN"
    fi
done

echo "═══════════════════════════════════════════════════════"
echo "           Beacon End-to-End Demo Complete!            "
echo "═══════════════════════════════════════════════════════"
