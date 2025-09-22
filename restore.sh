#!/bin/bash
###############################################################################
# @file        retrieve_ringbuffer.sh
# @brief       Restore spsc_ringbuffer.h from Git and place in Beacon/include
# @author      Bryan Camp
###############################################################################

set -e

# --- Configuration ---
COMMIT_HASH="5101a8cca821a8e5126f1ab54d76504cb2c6abb9"
OLD_PATH="src/common/spsc_ringbuffer.h"
NEW_DIR="Beacon/include/common"
NEW_PATH="$NEW_DIR/spsc_ringbuffer.h"

# --- Ensure project root ---
PROJECT_ROOT="$(pwd)"

echo "[INFO] Restoring $OLD_PATH from commit $COMMIT_HASH..."

# --- Checkout the file from the specified commit ---
git checkout $COMMIT_HASH -- $OLD_PATH

# --- Ensure destination directory exists ---
mkdir -p $NEW_DIR

# --- Move file to new directory ---
mv $OLD_PATH $NEW_PATH

echo "[INFO] File moved to $NEW_PATH"

# --- Confirm presence ---
if [[ -f "$NEW_PATH" ]]; then
    echo "[SUCCESS] spsc_ringbuffer.h successfully restored and placed in $NEW_DIR"
else
    echo "[ERROR] File not found after move"
    exit 1
fi
