#!/bin/bash
set -e

echo "[Step 1] Renaming main_* files to md_*.cpp"

# Rename the files
mv src/apps/nsdq/md_generator/main_generator.cpp src/apps/nsdq/md_generator/md_generator.cpp
mv src/apps/nsdq/md_listener/main_listener.cpp src/apps/nsdq/md_listener/md_listener.cpp
mv src/apps/nsdq/md_playback/main_replayer.cpp src/apps/nsdq/md_playback/md_replayer.cpp

echo "[Step 2] Removing old market_data folder if it exists"
if [ -d "src/apps/nsdq/market_data" ]; then
    rm -rf src/apps/nsdq/market_data
    echo "  market_data folder removed"
else
    echo "  market_data folder not found, skipping"
fi

echo "[Step 3] Updating CMakeLists.txt"

CMAKE_FILE="CMakeLists.txt"

# Use sed to replace the old executable entries
sed -i.bak -E 's|apps/nsdq/market_data/generator_main.cpp|apps/nsdq/md_generator/md_generator.cpp|g' $CMAKE_FILE
sed -i.bak -E 's|apps/nsdq/market_data/listener_main.cpp|apps/nsdq/md_listener/md_listener.cpp|g' $CMAKE_FILE
sed -i.bak -E 's|apps/nsdq/market_data/replayer_main.cpp|apps/nsdq/md_playback/md_replayer.cpp|g' $CMAKE_FILE

echo "[Step 4] Done! Backup of original CMakeLists.txt saved as CMakeLists.txt.bak"
