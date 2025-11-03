#!/usr/bin/env python3
"""
@project     Beacon
@component   Build System [Top-Level]
@file        build.py
@brief       User-facing build script for debug/release/all configurations
@author      Bryan Camp

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

import os
import sys
import shutil
import subprocess
import glob
from pathlib import Path

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

PROJECT_ROOT = "/Users/bryancamp/SoftwareDevelopment/cpp/professional/Beacon"
BUILD_DIR = os.path.join(PROJECT_ROOT, "build/debug")
GENERATOR_DIR = os.path.join(PROJECT_ROOT, "src/apps/md_generator")


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


def run(cmd, cwd=None, check=True):
    print(f"> {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if check and result.returncode != 0:
        sys.exit(result.returncode)


def main():
    os.makedirs(BUILD_DIR, exist_ok=True)

    # Configure CMake for debug build
    run([
        "cmake",
        "-DCMAKE_BUILD_TYPE=Debug",
        PROJECT_ROOT
    ], cwd=BUILD_DIR)

    # Lint check with clang-format (does not modify files, just checks)
    lint_failed = False
    for ext in ("*.cpp", "*.h"):
        for f in glob.glob(os.path.join(GENERATOR_DIR, ext)):
            print(f"Linting {f} ...")
            result = subprocess.run(["clang-format", "--dry-run", "--Werror", f])
            if result.returncode != 0:
                print(f"clang-format failed for {f}")
                lint_failed = True
    if lint_failed:
        print("Lint check failed. Please fix formatting issues before building.")
        sys.exit(1)

    # Static analysis with clang-tidy
    tidy_failed = False
    for f in glob.glob(os.path.join(GENERATOR_DIR, "*.cpp")):
        print(f"Running clang-tidy on {f} ...")
        result = subprocess.run([
            "clang-tidy", f, "--", "-std=c++20"
        ])
        if result.returncode != 0:
            print(f"clang-tidy failed for {f}")
            tidy_failed = True
    if tidy_failed:
        print("clang-tidy check failed. Please fix issues before building.")
        sys.exit(1)

    # Build only md_generator
    run(["cmake", "--build", ".", "--target", "md_generator"], cwd=BUILD_DIR)

if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        print(f"Build failed: {e}", file=sys.stderr)
        sys.exit(1)
