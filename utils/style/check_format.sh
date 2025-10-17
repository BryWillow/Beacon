#!/usr/bin/env bash
# @file check_format.sh
# Simple script to check formatting using clang-format
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CLANG_FORMAT="clang-format"

echo "Running clang-format check..."
FAILED=0

find "$ROOT" -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -not -path "*/vendor/*" | while read -r file; do
    "$CLANG_FORMAT" -style=file "$file" | diff -u "$file" - >/dev/null || {
        echo "Formatting mismatch: $file"
        FAILED=1
    }
done

if [ "$FAILED" -ne 0 ]; then
    echo "Some files are not formatted. Run 'clang-format -i' to fix or use your editor integration."
    exit 1
fi

echo "All files formatted according to .clang-format"
