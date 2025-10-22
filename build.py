#!/usr/bin/env python3
"""
@file        build.py
@project     Beacon
@component   Build System
@brief       User-facing build script for debug/release/all configurations
@author      Bryan Camp
@date        2024
@copyright   Copyright (c) 2024 Bryan Camp. All rights reserved.

@details
User-facing build script for the Beacon project.

Usage:
  ./build.py              # Build debug (default)
  ./build.py debug        # Build debug explicitly
  ./build.py release      # Build release
  ./build.py all          # Build both debug and release
  ./build.py clean        # Clean build artifacts

Exit codes:
  0 = success
  1 = build/validation error
  2 = interrupted by user (Ctrl+C)
"""

import sys
from pathlib import Path
import subprocess
import shutil
import glob

# Ensure internal pipeline is importable
_pipeline = Path(__file__).parent / "scripts" / "internal" / "cicd_pipeline"
if str(_pipeline) not in sys.path:
    sys.path.insert(0, str(_pipeline))
try:
    from _build_utils import execute_build
except Exception as e:
    print(f"ERROR: Failed to import build utilities: {e}", file=sys.stderr)
    print(f"Searched in: {_pipeline}", file=sys.stderr)
    sys.exit(1)


def get_repo_root() -> Path:
    """Resolve repository root using git."""
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True,
            text=True,
            check=True
        )
        return Path(result.stdout.strip())
    except subprocess.CalledProcessError as e:
        print("ERROR: Not a git repository", file=sys.stderr)
        sys.exit(1)


def _remove_path(p: Path) -> None:
    try:
        if p.is_dir():
            shutil.rmtree(p)
            print(f"Removed: {p}")
        elif p.exists():
            p.unlink()
            print(f"Removed: {p}")
    except Exception as e:
        print(f"Warning: failed to remove {p}: {e}", file=sys.stderr)


def clean(repo_root: Path, deep: bool = False) -> None:
    """
    Clean build artifacts.
    - Always removes: bin/, build_debug/, build_release/, build_Debug/, build_Release/
    - Deep clean also removes: any build_* dirs and CMake cache/files under them.
    """
    # Known directories (lowercase + legacy uppercase)
    known_dirs = [
        repo_root / "bin",
        repo_root / "build_debug",
        repo_root / "build_release",
        repo_root / "build_Debug",
        repo_root / "build_Release",
    ]
    for d in known_dirs:
        if d.exists():
            _remove_path(d)

    if deep:
        # Remove any stray build_* dirs at repo root
        for path in repo_root.glob("build_*"):
            if path.is_dir():
                _remove_path(path)

        # Remove common CMake cache files wherever they might have leaked
        for pat in [
            "**/CMakeCache.txt",
            "**/CMakeFiles",
            "**/.cmake/api",
        ]:
            for match in repo_root.glob(pat):
                _remove_path(match if match.is_dir() else match)


def main():
    """Main entry point."""
    repo_root = get_repo_root()

    # Parse arguments
    config = sys.argv[1] if len(sys.argv) > 1 else "debug"

    # Handle clean modes
    if config in {"clean", "deep-clean"}:
        print("Cleaning build artifacts..." + (" (deep)" if config == "deep-clean" else ""))
        clean(repo_root, deep=(config == "deep-clean"))
        return

    # Validate configuration
    valid = {"debug", "release", "all"}
    if config not in valid:
        print(f"ERROR: Invalid config '{config}'. Must be: {', '.join(valid)}", file=sys.stderr)
        sys.exit(1)

    # Build
    if config == "all":
        build_modes = ["debug", "release"]
    else:
        build_modes = [config]

    for mode in build_modes:
        try:
            execute_build(mode, repo_root)
        except subprocess.CalledProcessError:
            print(f"ERROR: {mode} build failed.", file=sys.stderr)
            sys.exit(1)

    print("✓ Build complete")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr)
        sys.exit(2)
