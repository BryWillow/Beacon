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


def main():
    """Main entry point."""
    repo_root = get_repo_root()

    # Parse arguments
    config = sys.argv[1] if len(sys.argv) > 1 else "debug"

    # Handle clean
    if config == "clean":
        print("Cleaning build artifacts...")
        for d in ["bin", "build_debug", "build_release"]:
            path = repo_root / d
            if path.exists():
                import shutil
                shutil.rmtree(path)
                print(f"Removed: {path}")
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
