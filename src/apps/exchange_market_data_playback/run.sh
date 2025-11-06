#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_playback
# Purpose:      Run script for the market data playback
# Author:       Bryan Camp
# =============================================================================

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="${SCRIPT_DIR}/build/exchange_market_data_playback"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "[run.sh] Error: Binary not found at ${BINARY}"
    echo "[run.sh] Please run ./build.sh first"
    exit 1
fi

# Check if input file provided
if [ $# -lt 1 ]; then
    echo "[run.sh] Usage: ./run.sh <input_file> [--config <config.json>] [--wait-for-ready]"
    echo "[run.sh] Example: ./run.sh output.itch"
    echo "[run.sh] Example: ./run.sh output.itch --config playback_config.json --wait-for-ready"
    exit 1
fi

echo "[run.sh] Running exchange_market_data_playback"
echo "[run.sh] Arguments: $@"
echo ""

"$BINARY" "$@"

echo ""
echo "[run.sh] Playback complete!"
