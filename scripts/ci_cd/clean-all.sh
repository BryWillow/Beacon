#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "[CI/CD] Cleaning all build and log artifacts..."

rm -rf "$REPO_ROOT/src/apps/exchange_matching_engine/build-debug" \
       "$REPO_ROOT/src/apps/exchange_matching_engine/build-release" \
       "$REPO_ROOT/src/apps/exchange_market_data_generator/build-debug" \
       "$REPO_ROOT/src/apps/exchange_market_data_generator/build-release" \
       "$REPO_ROOT/src/apps/exchange_market_data_playback/build-debug" \
       "$REPO_ROOT/src/apps/exchange_market_data_playback/build-release" \
       "$REPO_ROOT/src/apps/client_algorithm/build-debug" \
       "$REPO_ROOT/src/apps/client_algorithm/build-release" \
       "$REPO_ROOT/logs/" \
       "$REPO_ROOT/scripts/logs/"

echo "[CI/CD] Clean complete!"
