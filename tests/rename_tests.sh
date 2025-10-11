#!/bin/bash
# ---------------------------------------------------------------------------
# @file        rename_tests.sh
# @description Remove 'test_' prefix from test directories and source files
# ---------------------------------------------------------------------------

set -euo pipefail

TESTS_DIR="$(pwd)"

echo "Renaming test directories and files in $TESTS_DIR ..."

# Step 1: Rename directories
find "$TESTS_DIR" -depth -type d -name 'test_*' | while read -r dir; do
    base=$(basename "$dir")
    parent=$(dirname "$dir")
    newname="${base#test_}"
    echo "Renaming directory $dir -> $parent/$newname"
    mv "$dir" "$parent/$newname"
done

# Step 2: Rename files
find "$TESTS_DIR" -type f -name 'test_*.cpp' | while read -r file; do
    base=$(basename "$file")
    dir=$(dirname "$file")
    newname="${base#test_}"
    echo "Renaming file $file -> $dir/$newname"
    mv "$file" "$dir/$newname"
done

# Step 3: Update CMakeLists.txt files
echo "Updating CMakeLists.txt paths..."
find "$TESTS_DIR" -type f -name 'CMakeLists.txt' | while read -r cmake; do
    echo "Processing $cmake"
    # replace test_ prefix in add_executable calls
    sed -i.bak 's|add_executable(test_\([a-zA-Z0-9_]*\)|add_executable(\1|g' "$cmake"
    # replace test_ prefix in add_test calls
    sed -i.bak 's|add_test(NAME test_\([a-zA-Z0-9_]*\)|add_test(NAME \1|g' "$cmake"
done

echo "Renaming complete. Backup CMakeLists.txt files with .bak created."
