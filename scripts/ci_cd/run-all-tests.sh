#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "[CI/CD] Running all tests..."

# Example: run a Python test runner, or C++ test binaries
python3 "$REPO_ROOT/scripts/beacon_test/beacon-run-unit-tests.py"

# Add more test runners as needed

echo "[CI/CD] All tests complete!"
