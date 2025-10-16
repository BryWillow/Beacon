#!/usr/bin/env bash
set -e

# ---------------------------------------------------------------------------
# @project   : Beacon Ecosystem
# @version   : 1.0.6       # Initial version; will auto-update
# @date      : $(date +%Y-%m-%d)
# @build_by  : Bryan Camp
# ---------------------------------------------------------------------------

REPO_ROOT="$(git rev-parse --show-toplevel)"
VERSION_FILE="$REPO_ROOT/VERSION"
RELEASE_NOTES="$REPO_ROOT/ReleaseNotes.txt"

cd "$REPO_ROOT"

# Check for uncommitted changes
if [[ -n $(git status --porcelain) ]]; then
    echo "Warning: You have uncommitted changes. Commit them before release."
    exit 1
fi

# Check for changes since last release
last_tag=$(git describe --tags --abbrev=0 2>/dev/null || echo "")
changes_since_last=$(git log ${last_tag}..HEAD --oneline)
if [[ -z "$changes_since_last" ]]; then
    echo "No changes since last release. Cannot create a release."
    exit 1
fi

# Load current version
if [[ -f "$VERSION_FILE" ]]; then
    IFS='.' read -r MAJOR MINOR PATCH BUILD < "$VERSION_FILE"
else
    MAJOR=1
    MINOR=0
    PATCH=0
    BUILD=0
fi

# Determine type of change
increment_patch=0
increment_minor=0
increment_major=0
while read -r hash msg; do
    if [[ $msg == *BREAKING* ]]; then
        increment_major=1
    elif [[ $msg == *ENHANCEMENT* ]]; then
        increment_minor=1
    else
        increment_patch=1
    fi
done <<< "$(git log ${last_tag}..HEAD --pretty=format:"%H %s")"

if [[ $increment_major -eq 1 ]]; then
    ((MAJOR++))
    MINOR=0
    PATCH=0
    BUILD=0
elif [[ $increment_minor -eq 1 ]]; then
    ((MINOR++))
    PATCH=0
    BUILD=0
elif [[ $increment_patch -eq 1 ]]; then
    ((PATCH++))
    BUILD=0
fi

# Increment build number
((BUILD++))
NEW_VERSION="${MAJOR}.${MINOR}.${PATCH}.${BUILD}"
echo "$NEW_VERSION" > "$VERSION_FILE"

# Gather last 500 commits
COMMITS=$(git log -n 500 --pretty=format:"%H %s")

# Generate ReleaseNotes.txt
tmpfile=$(mktemp)
{
    echo "# ---------------------------------------------------------------------------"
    echo "# @project   : Beacon Ecosystem"
    echo "# @version   : $NEW_VERSION"
    echo "# @date      : $(date +%Y-%m-%d)"
    echo "# @build_by  : Bryan Camp"
    echo "# ---------------------------------------------------------------------------"
    echo
    echo "This release contains $(git log ${last_tag}..HEAD --oneline | wc -l | tr -d ' ') updates. See below:"
    echo
    git log ${last_tag}..HEAD --pretty=format:"v$NEW_VERSION, %h, %s"
    echo
    echo "Last 500 commits:"
    while read -r hash msg; do
        echo "v$NEW_VERSION, $hash, $msg"
    done <<< "$COMMITS"
} > "$tmpfile"

# Open editor for tweaks
EDITOR="${EDITOR:-vi}"
$EDITOR "$tmpfile"

# Finalize ReleaseNotes.txt
cp "$tmpfile" "$RELEASE_NOTES"
rm "$tmpfile"

# Commit and tag
git add "$VERSION_FILE" "$RELEASE_NOTES"
commit_msg="Release $NEW_VERSION"
git commit -m "$commit_msg"
git tag "v$NEW_VERSION"

echo "Release $NEW_VERSION generated, committed, and tagged."
