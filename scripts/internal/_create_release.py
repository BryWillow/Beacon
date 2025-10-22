#!/usr/bin/env python3
"""
Release automation (maintainers only).
Builds all configs via build.py, validates artifacts, and (in real mode) would tag/push.
"""

import sys
import subprocess
from pathlib import Path


def repo_root() -> Path:
    res = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True, text=True, check=True
    )
    return Path(res.stdout.strip())


def build_all(root: Path, dry_run: bool) -> None:
    build_script = root / "build.py"
    if not build_script.exists():
        raise FileNotFoundError(f"Missing: {build_script}")
    cmd = [sys.executable, str(build_script), "all"]
    if dry_run:
        print("DRY-RUN:", " ".join(cmd))
        return
    subprocess.run(cmd, check=True, cwd=root)


def _expected_binaries(root: Path) -> list[Path]:
    return [
        root / "bin" / "file_generator" / "debug" / "file_generator",
        root / "bin" / "file_generator" / "release" / "file_generator",
    ]


def validate_outputs(root: Path, dry_run: bool) -> None:
    if dry_run:
        for p in _expected_binaries(root):
            print(f"DRY-RUN: validate exists -> {p.relative_to(root)}")
        return
    missing = [p for p in _expected_binaries(root) if not p.exists()]
    if missing:
        rel = ", ".join(str(p.relative_to(root)) for p in missing)
        raise RuntimeError(f"Missing artifacts: {rel}")


def main() -> int:
    dry_run = "--dry-run" in sys.argv
    root = repo_root()
    print("DRY-RUN: Creating release..." if dry_run else "Creating release...")
    build_all(root, dry_run=dry_run)
    validate_outputs(root, dry_run=dry_run)
    # ...existing code... (version bumping, notes, tagging)
    print("Release process completed successfully.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr)
        sys.exit(2)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
