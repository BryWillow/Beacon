#!/usr/bin/env bash
# Abstract build script for Beacon
# Delegates to app-specific build scripts in the `pipeline` directory.

set -euo pipefail

# ANSI codes for bold colors
BOLD="\033[1m"
YELLOW="\033[33m"
RED="\033[31m"
RESET="\033[0m"

# Function to display usage
usage() {
    echo "Usage: $0 [action]"
    echo "Actions: debug, release, all, clean"
    echo "Binaries are located in:"
    echo "  debug    -> bin/<app_name>/debug"
    echo "  release  -> bin/<app_name>/release"
    exit 1
}

# Ensure action is provided
if [ "$#" -lt 1 ]; then
    usage
fi

ACTION="$1"
REPO_ROOT="$(git rev-parse --show-toplevel)"
APPS=("file_generator" "udp_replayer" "matching_engine" "market_data_processor")

# Main case block
case "$ACTION" in
    debug|release|all|clean)
        for app in "${APPS[@]}"; do
            "${REPO_ROOT}/scripts/pipeline/build/build_${app}.sh" "$ACTION"
        done
        ;;
    *)
        echo -e "${BOLD}${RED}ERROR: Unknown build mode specified${RESET}"
        usage
        ;;
esac