#!/usr/bin/env python3
"""
Check for expected test sources and print actionable hints.
Run from repo root:
    ./check_tests.py
"""
import sys
from pathlib import Path

def find_repo_root(start: Path) -> Path:
    for p in [start] + list(start.parents):
        if (p / "CMakeLists.txt").exists() or (p / ".git").exists():
            return p
    return Path.cwd()

script_path = Path(__file__).resolve()
repo = find_repo_root(script_path.parent)
print(f"Using repo root: {repo}")

checks = [
    ("Itch test source", repo / "tests" / "test_itch_file_generator" / "itch_file_generator.cpp"),
    ("Smoke test source", repo / "tests" / "test_smoke" / "test_smoke.cpp"),
    ("Public ITCH header", repo / "include" / "nsdq" / "market_data" / "itch" / "messages_active.h"),
]

missing = []
for name, path in checks:
    if not path.exists():
        missing.append((name, path))

if not missing:
    print("OK: all expected test sources and headers present.")
    sys.exit(0)

print("ERROR: missing files:")
for name, path in missing:
    print(f"  - {name}: expected at {path}")
print("\nSuggestions:")
print("  - If files were moved, update the corresponding CMakeLists.txt to point to the correct path.")
print("  - If files are generated, run the generator step before configuring CMake.")
print("  - If a test is obsolete, remove its add_subdirectory/add_executable entry from tests/CMakeLists.txt.")
sys.exit(2)
