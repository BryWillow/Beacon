#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   : Beacon Ecosystem
# @version   :
# @date      :
# @build_by  : Bryan Camp
# ---------------------------------------------------------------------------

REPO_ROOT="$(git rev-parse --show-toplevel)"
VERSION_FILE="$REPO_ROOT/VERSION"
RELEASE_NOTES="$REPO_ROOT/ReleaseNotes.txt"

# Go to repo root
cd "$REPO_ROOT"

# 1. Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
    echo "WARNING: You have uncommitted changes. Please commit or stash them before releasing."
fi

# 2. Ensure VERSION file exists
if [[ ! -f "$VERSION_FILE" ]]; then
    echo "0.0.0.0" > "$VERSION_FILE"
fi

VERSION=$(cat "$VERSION_FILE")
IFS='.' read -r MAJOR MINOR PATCH BUILD <<< "$VERSION"

# 3. Detect what type of update this is (simplified placeholder)
#    This can later be replaced with commit message analysis
#    For now, just increment build
BUILD=$((BUILD + 1))
NEW_VERSION="$MAJOR.$MINOR.$PATCH.$BUILD"

# 4. Prepare ReleaseNotes.txt header
CURRENT_DATE=$(date +%Y-%m-%d)
cat > "$RELEASE_NOTES" <<EOF
# ---------------------------------------------------------------------------
# @project   : Beacon Ecosystem
# @version   : $NEW_VERSION
# @date      : $CURRENT_DATE
# @build_by  : Bryan Camp
# ---------------------------------------------------------------------------

This release contains 0 updates. See below:

# Instructions (edit only if desired):
# To commit this release notes:
#   git add ReleaseNotes.txt
#   git commit -m "Release notes for $NEW_VERSION"
#   git tag -a "v$NEW_VERSION" -m "Tag for release $NEW_VERSION"
EOF

# 5. Detect commits included in this release
#    For simplicity, use all commits since last tag
LAST_TAG=$(git describe --tags --abbrev=0 || echo "")
if [[ -z "$LAST_TAG" ]]; then
    COMMITS=$(git log -500 --pretty=format:"v$NEW_VERSION, %h, %s")
else
    COMMITS=$(git log "$LAST_TAG"..HEAD --pretty=format:"v$NEW_VERSION, %h, %s" -500)
fi

# 6. Add commits to ReleaseNotes.txt
if [[ -z "$COMMITS" ]]; then
    echo "---- No commits detected ----" >> "$RELEASE_NOTES"
else
    echo "$COMMITS" >> "$RELEASE_NOTES"
fi

# 7. Write ReleaseNotes.txt to debug and release dirs
for mode in debug release; do
    DEST="$REPO_ROOT/bin/file_generator/$mode/ReleaseNotes.txt"
    mkdir -p "$(dirname "$DEST")"
    cp "$RELEASE_NOTES" "$DEST"
done

# 8. Update VERSION file
echo "$NEW_VERSION" > "$VERSION_FILE"

# 9. Summary message
echo ""
echo "Prepared ReleaseNotes.txt for version $NEW_VERSION."
echo "Review the file and edit only the notes/comments if desired."
echo "Instructions for committing and tagging are included as comments in the file."
