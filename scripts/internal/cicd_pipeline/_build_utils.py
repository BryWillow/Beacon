#!/usr/bin/env python3
"""
@file        _build_utils.py
@brief       Shared CMake build utilities
@details     Low-level CMake execution wrapper used by build.py and _create_release.py
"""

import subprocess
from pathlib import Path


def execute_build(mode: str, repo_root: Path) -> None:
    """
    Execute CMake configure and build for the specified mode.

    Args:
        mode: "debug" or "release"
        repo_root: Repository root path

    Raises:
        subprocess.CalledProcessError: If build fails
    """
    if mode not in {"debug", "release"}:
        raise ValueError(f"Invalid build mode: {mode}")

    build_type = mode.capitalize()  # Debug or Release
    # Use lowercase build dirs to match repo convention
    build_dir = repo_root / f"build_{mode}"

    # Quieter configure: suppress dev warnings and rule messages
    configure_cmd = [
        "cmake",
        "-S", str(repo_root),
        "-B", str(build_dir),
        f"-DCMAKE_BUILD_TYPE={build_type}",
        # Quieter configure
        "-DCMAKE_RULE_MESSAGES=OFF",
        "-DCMAKE_MESSAGE_LOG_LEVEL=WARNING",
        "-Wno-dev",
    ]
    print(f"Configuring {mode}...")
    subprocess.run(configure_cmd, check=True, cwd=repo_root)

    # Build (keep default non-verbose)
    build_cmd = [
        "cmake",
        "--build", str(build_dir),
        "-j",
    ]
    print(f"Building {mode}...")
    subprocess.run(build_cmd, check=True, cwd=repo_root)
    print(f"✓ {mode} build complete")
