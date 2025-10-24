```bash
#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [md_client]
# @file      md_client.sh
# @brief     Build script for the md_client application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# Fail fast
set -euo pipefail

# ---------------------------------------------------------------------------
# Variables
# ---------------------------------------------------------------------------
APP_NAME="md_client"
APP_PATH="src/apps/md_client"

# ---------------------------------------------------------------------------
# Include common functions
# ---------------------------------------------------------------------------
# Ensure internal pipeline is importable
_pipeline="$(dirname "$(realpath "$0")")/../scripts/internal/cicd_pipeline"
if [[ ! -f "${_pipeline}/_build_utils.sh" ]]; then
    echo "ERROR: Build utilities not found: ${_pipeline}/_build_utils.sh" >&2
    exit 1
fi
source "${_pipeline}/_build_utils.sh"

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
function main() {
    # Parse arguments
    local config="${1:-debug}"

    # Validate configuration
    if [[ ! "$config" =~ ^(debug|release)$ ]]; then
        echo "ERROR: Invalid config '$config'. Must be: debug, release" >&2
        exit 1
    fi

    # Build
    execute_build "$config" "$(get_repo_root)"
}

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
main "$@"
```