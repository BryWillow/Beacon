#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

APPS=(
    "exchange_matching_engine"
    "exchange_market_data_generator"
    "exchange_market_data_playback"
    "client_algorithm"
)

for APP in "${APPS[@]}"; do
    echo "[$APP] Building (Release)..."
    bash "$REPO_ROOT/src/apps/$APP/build-release.sh"
done

echo "[CI/CD] All release builds complete!"
