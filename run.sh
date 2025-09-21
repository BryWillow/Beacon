#!/bin/bash
###############################################################################
# @file        run.sh
# @author      Bryan Camp
# @brief       Launcher script for Market Data Pipeline
# @details     Starts listener first, then replayer.
#              All output is logged under logs/.
###############################################################################

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN_DIR="$PROJECT_ROOT/bin/Debug"
LOG_DIR="$PROJECT_ROOT/logs"

LISTENER_BIN="$BIN_DIR/nsdq_listener"
REPLAYER_BIN="$BIN_DIR/nsdq_replayer"

#==============================#
# Prepare environment
#==============================#
mkdir -p "$LOG_DIR"

if [[ ! -x "$LISTENER_BIN" ]]; then
    echo "Error: Listener binary not found at $LISTENER_BIN"
    exit 1
fi

if [[ ! -x "$REPLAYER_BIN" ]]; then
    echo "Error: Replayer binary not found at $REPLAYER_BIN"
    exit 1
fi

#==============================#
# Start listener
#==============================#
echo "[Run] Starting listener..."
"$LISTENER_BIN" > "$LOG_DIR/listener.log" 2>&1 &
LISTENER_PID=$!

# Give listener time to bind to socket (tweak if needed)
sleep 1

#==============================#
# Start replayer
#==============================#
echo "[Run] Starting replayer..."
"$REPLAYER_BIN" > "$LOG_DIR/replayer.log" 2>&1 &
REPLAYER_PID=$!

echo "[Run] Listener PID: $LISTENER_PID"
echo "[Run] Replayer PID: $REPLAYER_PID"
echo "[Run] Logs: $LOG_DIR"

#==============================#
# Wait for processes
#==============================#
wait $LISTENER_PID
wait $REPLAYER_PID

