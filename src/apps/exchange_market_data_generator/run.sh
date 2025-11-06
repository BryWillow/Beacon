#!/bin/bash
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_generator
# Purpose:      Run script for the market data generator
# Author:       Bryan Camp
# =============================================================================

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="${SCRIPT_DIR}/build/exchange_market_data_generator"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "[run.sh] Error: Binary not found at ${BINARY}"
    echo "[run.sh] Please run ./build.sh first"
    exit 1
fi

# Default config and output if not provided
CONFIG_FILE="${1:-${SCRIPT_DIR}/sample_config.json}"
OUTPUT_FILE="${2:-output.itch}"

# Check if config file exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo "[run.sh] Error: Config file not found: ${CONFIG_FILE}"
    echo "[run.sh] Usage: ./run.sh [config_file] [output_file]"
    echo "[run.sh] Example: ./run.sh sample_config.json output.itch"
    exit 1
fi

"$BINARY" "$CONFIG_FILE" "$OUTPUT_FILE"
