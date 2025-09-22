#!/usr/bin/env bash
set -e

PROJECT_ROOT="$(pwd)"
INCLUDE_DIR="$PROJECT_ROOT/include"
NSDQ_DIR="$INCLUDE_DIR/nsdq"

echo "[INFO] Organizing NSDQ headers in $INCLUDE_DIR ..."

# List of NSDQ headers you want to move
NSDQ_HEADERS=(
    "itch_message.h"
    "itch_udp_listener.h"
    "itch_udp_replayer.h"
)

for header in "${NSDQ_HEADERS[@]}"; do
    if [[ -f "$INCLUDE_DIR/$header" ]]; then
        mv "$INCLUDE_DIR/$header" "$NSDQ_DIR/"
        echo "[Move] $header -> nsdq/"
    else
        echo "[Skip] $header not found in $INCLUDE_DIR"
    fi
done

# Remove empty files (optional)
find "$NSDQ_DIR" -type f -size 0 -exec rm -v {} \;

echo "[Done] NSDQ headers organized."
