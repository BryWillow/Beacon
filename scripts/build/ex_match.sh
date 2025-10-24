```bash
#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# @project   Beacon
# @component Build Script [App: ex_match]
# @file      ex_match.sh
# @brief     Build script for the ex_match application
# @author    Bryan Camp
# ---------------------------------------------------------------------------

# Application-specific settings
APP_NAME="ex_match"
APP_PATH="src/apps/ex_match"

# Include common build functions
SCRIPT_DIR="$(dirname "$(realpath "$0")")"
source "${SCRIPT_DIR}/../_build_utils.sh"

# Execute the build
execute_build "$APP_NAME" "$APP_PATH"
```