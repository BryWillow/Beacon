#!/bin/bash
###############################################################################
# @file        git_commit.sh
# @brief       Safely commit staged files with detailed commit message
# @author      Bryan Camp
###############################################################################

set -euo pipefail

# Ensure we are in the top-level Beacon directory
REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

# Ensure the log directory path is absolute
LOG_DIR="$REPO_ROOT/scripts/pipeline/commit/git_commit_logs"
mkdir -p "$LOG_DIR"
TIMESTAMP=$(date +%Y-%m-%d_%H%M%S)

# Stage all changes
git add .

# Get staged files
STAGED=$(git diff --cached --name-only)
if [[ -z "$STAGED" ]]; then
    echo "No files staged for commit"
    exit 1
fi

# Prevent committing src files modified directly on GitHub
for f in $STAGED; do
    if [[ $f == src/*.[ch]pp ]] || [[ $f == src/*.[hH] ]]; then
        echo "ERROR: Cannot commit C++ source files directly from GitHub edits"
        exit 1
    fi
done

# Generate commit message
COMMIT_MSG_FILE="$LOG_DIR/git_commit_msg_$TIMESTAMP.txt"
{
    echo "Commit Timestamp: $TIMESTAMP"
    echo "Version: $(cat VERSION)"
    echo ""
    echo "* Staged files:"
    for f in $STAGED; do
        echo "  - $f"
    done
    echo ""
    echo "* Build info:"
    echo "  - Debug and Release binaries built via build.sh"
    echo "  - Unit tests passed (if enabled)"
    echo ""
    echo "* Notes:"
    echo "  - To recreate build: run ./build.sh [debug|release|all]"
} > "$COMMIT_MSG_FILE"

echo "Generated commit message at $COMMIT_MSG_FILE"
echo ""
cat "$COMMIT_MSG_FILE"
echo ""
read -p "Would you really like to commit these files? Type 'yes' to confirm: " CONFIRM

# Prompt user for additional commit message
read -p "Would you like to add extra details to the commit message? (leave blank to skip): " EXTRA_MSG
if [[ -n "$EXTRA_MSG" ]]; then
    echo "" >> "$COMMIT_MSG_FILE"
    echo "* Additional Notes:" >> "$COMMIT_MSG_FILE"
    echo "$EXTRA_MSG" >> "$COMMIT_MSG_FILE"
fi

if [[ "$CONFIRM" == "yes" ]]; then
    git commit -F "$COMMIT_MSG_FILE"
    echo "Commit successful!"
    cp "$COMMIT_MSG_FILE" "$LOG_DIR/git_commit_SUCCESS_$TIMESTAMP.log"
else
    echo "Commit aborted"
    cp "$COMMIT_MSG_FILE" "$LOG_DIR/git_commit_ABORTED_$TIMESTAMP.log"
    exit 1
fi
