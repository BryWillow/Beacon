#!/usr/bin/env bash
set -euo pipefail

CMAKE_FILE="./CMakeLists.txt"

if [[ ! -f "$CMAKE_FILE" ]]; then
    echo "[ERROR] CMakeLists.txt not found at $CMAKE_FILE"
    exit 1
fi

echo "[INFO] Updating CMakeLists.txt ..."

# Use sed to replace old headers with new ones
sed -i.bak -e 's|itch_message.h|md_itch_message.h|g' \
           -e 's|itch_file_generator.h|md_itch_generator.h|g' \
           -e 's|itch_message_udp_replayer.h|md_itch_udp_replayer.h|g' \
           -e 's|itch_udp_listener.h|md_itch_udp_listener.h|g' "$CMAKE_FILE"

rm -f "$CMAKE_FILE.bak"

echo "[DONE] CMakeLists.txt update complete."
