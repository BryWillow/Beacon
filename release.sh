#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   : Beacon Ecosystem
# @version   : 1.0.0.0  # placeholder, updated automatically
# @date      : YYYY-MM-DD # placeholder, updated automatically
# @build_by  : Bryan Camp
# ---------------------------------------------------------------------------

# Configuration
REPO_ROOT="$(git rev-parse --show-toplevel)"
VERSION_FILE="${REPO_ROOT}/VERSION"
RELEASE_NOTES_FILE="ReleaseNotes.txt"
MAX_COMMITS=500
ASCII_LINE="------------------------------------------------------------"

# Helper: get current version
get_version() {
    [[ -f "$VERSION_FILE" ]] || echo "0.0.0.0" > "$VERSION_FILE"
    cat "$VERSION_FILE"
}

# Helper: increment build number
increment_build() {
    IFS='.' read -r MAJOR MINOR PATCH BUILD < "$VERSION_FILE"
    BUILD=$((BUILD + 1))
    echo "$MAJOR.$MINOR.$PATCH.$BUILD" > "$VERSION_FILE"
    echo "$MAJOR.$MINOR.$PATCH.$BUILD"
}

# 1. Ensure local changes are committed
if [[ -n "$(git status --porcelain)" ]]; then
    echo "WARNING: You have uncommitted changes. Commit before generating release notes."
    read -p "Continue anyway? (y/N) " confirm
    [[ "$confirm" == "y" || "$confirm" == "Y" ]] || exit 1
fi

# 1b. Ensure there are new commits since the last release
LAST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "")
if [[ -n "$LAST_TAG" ]]; then
    NEW_COMMITS=$(git log "$LAST_TAG"..HEAD --oneline)
    if [[ -z "$NEW_COMMITS" ]]; then
        echo "No new commits since the last release ($LAST_TAG). Cannot create a new release."
        exit 1
    fi
fi

# 2. Determine release type (major.minor.patch.build)
VERSION=$(get_version)
IFS='.' read -r MAJOR MINOR PATCH BUILD <<< "$VERSION"

# Placeholder: implement logic to detect breaking / enhancement / fix
# For now, just increment build number for every release
VERSION=$(increment_build)

# 3. Update header in ReleaseNotes.txt
DATE=$(date +"%Y-%m-%d")
cat > "$RELEASE_NOTES_FILE" <<EOL
# ---------------------------------------------------------------------------
# @project   : Beacon Ecosystem
# @version   : $VERSION
# @date      : $DATE
# @build_by  : Bryan Camp
# ---------------------------------------------------------------------------

This release contains 0 updates. See below:

EOL

# 4. Add last $MAX_COMMITS commits
git log -n "$MAX_COMMITS" --pretty=format:"v$VERSION, %h, %s" >> "$RELEASE_NOTES_FILE"

# 5. Separate each release with ASCII line
echo >> "$RELEASE_NOTES_FILE"
echo "$ASCII_LINE" >> "$RELEASE_NOTES_FILE"
echo >> "$RELEASE_NOTES_FILE"

# 6. Copy ReleaseNotes.txt to debug/release directories
for CONFIG in debug release; do
    BIN_DIR="${REPO_ROOT}/bin/file_generator/${CONFIG}"
    mkdir -p "$BIN_DIR"
    cp "$RELEASE_NOTES_FILE" "$BIN_DIR/$RELEASE_NOTES_FILE"
done

# 7. Optional: commit release notes
echo
echo "Prepared ReleaseNotes.txt for version $VERSION."
echo "Review the file and commit if desired:"
echo "  git add ReleaseNotes.txt"
echo "  git commit -m \"Release notes for $VERSION\""
