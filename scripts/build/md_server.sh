```bash
#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [md_server]
# @file      md_server.sh
# @brief     Build script for the md_server application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# Fail on any error
set -euo pipefail

# ---------------------------------------------------------------------------
# Variables
# ---------------------------------------------------------------------------
APP_NAME="md_server"
APP_PATH="src/apps/md_server"

# ---------------------------------------------------------------------------
# Include common build functions
# ---------------------------------------------------------------------------
# Ensure internal pipeline is importable
_pipeline="$(dirname "$0")/../scripts/internal/cicd_pipeline"
if [[ ! " ${BASH_SOURCE[@]} " =~ " ${_pipeline} " ]]; then
    source "${_pipeline}/_build_utils.sh"
fi

# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------
function build_md_server() {
    local mode="$1"
    local repo_root="$2"

    echo "Building ${APP_NAME} (${mode})..."

    # Example build commands (replace with actual build steps)
    if [[ "$mode" == "debug" ]]; then
        cmake -S "${APP_PATH}" -B "build/${APP_NAME}_debug" -DCMAKE_BUILD_TYPE=Debug
        cmake --build "build/${APP_NAME}_debug" --target "${APP_NAME}" -- -j "$(nproc)"
    elif [[ "$mode" == "release" ]]; then
        cmake -S "${APP_PATH}" -B "build/${APP_NAME}_release" -DCMAKE_BUILD_TYPE=Release
        cmake --build "build/${APP_NAME}_release" --target "${APP_NAME}" -- -j "$(nproc)"
    fi

    echo "✓ ${APP_NAME} built successfully"
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
function main() {
    local repo_root
    repo_root=$(get_repo_root)

    # Parse arguments
    local config
    config="${1:-debug}"

    # Build
    build_md_server "$config" "$repo_root"
}

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
main "$@"
```