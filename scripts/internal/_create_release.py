#!/usr/bin/env python3
"""
Release automation (maintainers only).
Builds all configs via build.py, validates artifacts, and (in real mode) would tag/push.
"""

import sys
import subprocess
import argparse
import re
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


SEMVER_RE = re.compile(r"^v(\d+)\.(\d+)\.(\d+)$")


def _run(cmd: list[str], cwd: Path) -> str:
    res = subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True)
    return res.stdout.strip()


def get_latest_tag(root: Path) -> str | None:
    try:
        return _run(["git", "describe", "--tags", "--abbrev=0"], root)
    except subprocess.CalledProcessError:
        return None


def parse_semver(tag: str) -> tuple[int, int, int]:
    m = SEMVER_RE.match(tag)
    if not m:
        raise ValueError(f"Invalid version tag format: {tag} (expected v<major>.<minor>.<patch>)")
    return tuple(map(int, m.groups()))


def bump_version(cur: str | None, bump: str) -> str:
    if cur is None:
        base = (0, 0, 0)
    else:
        base = parse_semver(cur)
    major, minor, patch = base
    if bump == "major":
        return f"v{major+1}.0.0"
    if bump == "minor":
        return f"v{major}.{minor+1}.0"
    if bump == "patch":
        return f"v{major}.{minor}.{patch+1}"
    raise ValueError(f"Unknown bump: {bump}")


def detect_change_type(root: Path) -> str:
    """
    Detect change type by diffing staged files vs. previous commit.
    Returns: 'major', 'minor', or 'patch'
    """
    # Get staged diff
    diff = _run(["git", "diff", "--cached", "--name-status", "HEAD"], root)
    files = [line.split('\t')[-1] for line in diff.splitlines() if line]
    # Major: any change in public API headers
    if any(f.startswith("include/") for f in files):
        return "major"
    # Minor: any new feature in src/ (added files)
    if any(f.startswith("src/") and line.startswith("A") for line in diff.splitlines()):
        return "minor"
    # Patch: everything else
    return "patch"


def ensure_tag_available(root: Path, tag: str) -> None:
    try:
        _run(["git", "rev-parse", "-q", "--verify", f"refs/tags/{tag}"], root)
        raise RuntimeError(f"Tag already exists: {tag}")
    except subprocess.CalledProcessError:
        return  # ok


def create_and_push_tag(root: Path, tag: str, message: str, force: bool) -> None:
    if not force:
        ensure_tag_available(root, tag)
    tag_cmd = ["git", "tag", "-a", tag, "-m", message]
    if force:
        tag_cmd.insert(2, "-f")
    _run(tag_cmd, root)
    _run(["git", "push", "origin", tag] + (["-f"] if force else []), root)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Create a release: build → validate → tag → push")
    g = p.add_mutually_exclusive_group()
    g.add_argument("--patch", action="store_true", help="Bump patch version")
    g.add_argument("--minor", action="store_true", help="Bump minor version")
    g.add_argument("--major", action="store_true", help="Bump major version")
    g.add_argument("--version", type=str, help="Explicit version tag (vX.Y.Z)")
    p.add_argument("--dry-run", action="store_true", help="Show actions only")
    p.add_argument("--force", action="store_true", help="Overwrite existing tag if present")
    p.add_argument("message", type=str, help="Annotated tag message")
    return p.parse_args()


def main() -> int:
    args = parse_args()
    dry_run = args.dry_run
    root = repo_root()
    print("DRY-RUN: Creating release..." if dry_run else "Creating release...")

    # 1) Build all and 2) validate
    build_all(root, dry_run=dry_run)
    validate_outputs(root, dry_run=dry_run)

    # 3) Resolve version
    if args.version:
        if not SEMVER_RE.match(args.version):
            raise ValueError("Version must be v<major>.<minor>.<patch>")
        next_tag = args.version
    else:
        # If user specified bump, use it; else auto-detect
        if args.patch:
            bump = "patch"
        elif args.minor:
            bump = "minor"
        elif args.major:
            bump = "major"
        else:
            bump = detect_change_type(root)
            print(f"Auto-detected version bump: {bump}")
        latest = get_latest_tag(root)
        next_tag = bump_version(latest, bump)

    if dry_run:
        print(f"DRY-RUN: would create and push tag -> {next_tag!r} with message: {args.message!r}")
        print("Release process completed successfully.")
        return 0

    # 4) Create and push tag
    create_and_push_tag(root, next_tag, args.message, args.force)
    print(f"Tagged and pushed: {next_tag}")
    print("Release process completed successfully.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("Interrupted by user.", file=sys.stderr); sys.exit(2)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr); sys.exit(1)
