#!/usr/bin/env python3
"""
Release automation script for Beacon (maintainers only).

Main responsibilities:
- Build all binaries via build.py
- Validate expected release artifacts
- Auto-detect version bump (major/minor/patch) based on staged changes, or accept manual override
- Create annotated git tag and push to origin
- Supports dry-run mode (shows actions, does not tag/push)
- Supports --force to overwrite existing tags

Usage examples:
    scripts/internal/_create_release.py --patch "Bugfix release"
    scripts/internal/_create_release.py --minor "Add new feature"
    scripts/internal/_create_release.py --major "Breaking changes"
    scripts/internal/_create_release.py --version v2.0.0 "Explicit version"
    scripts/internal/_create_release.py --patch "Preview" --dry-run

See copilot-instructions.md for full workflow details.
"""

import sys
import subprocess
import argparse
import re
from pathlib import Path
import shutil


def repo_root() -> Path:
    """
    Returns the absolute path to the git repository root.
    Used to resolve all other paths.
    """
    res = subprocess.run(
        ["git", "rev-parse", "--show-toplevel"],
        capture_output=True, text=True, check=True
    )
    return Path(res.stdout.strip())


def build_all(root: Path, dry_run: bool) -> None:
    """
    Delegates the build to build.py at repo root.
    Always builds both debug and release configs.
    """
    build_script = root / "build.py"
    if not build_script.exists():
        raise FileNotFoundError(f"Missing: {build_script}")
    cmd = [sys.executable, str(build_script), "all"]
    if dry_run:
        print("DRY-RUN:", " ".join(cmd))
        return
    subprocess.run(cmd, check=True, cwd=root)


def _expected_binaries(root: Path) -> list[Path]:
    """
    Returns a list of all expected release artifacts for all production-ready apps.
    """
    apps = [
        "file_generator",
        "udp_replayer",
        "matching_engine",
        # Add more app names here as they become production-ready
    ]
    configs = ["debug", "release"]
    binaries = []
    for app in apps:
        for config in configs:
            binaries.append(root / "bin" / app / config / app)
    return binaries


def validate_outputs(root: Path, dry_run: bool) -> None:
    """
    Ensures all required binaries exist after the build.
    Blocks release if any are missing.
    """
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
    """
    Helper to run a shell command and return its stdout.
    Used for git queries and tag creation.
    """
    res = subprocess.run(cmd, cwd=cwd, check=True, capture_output=True, text=True)
    return res.stdout.strip()


def get_latest_tag(root: Path) -> str | None:
    """
    Returns the latest git tag (semver), or None if no tags exist.
    Used to determine current version for bumping.
    """
    try:
        return _run(["git", "describe", "--tags", "--abbrev=0"], root)
    except subprocess.CalledProcessError:
        return None


def parse_semver(tag: str) -> tuple[int, int, int]:
    """
    Parses a semver tag (vX.Y.Z) into (major, minor, patch).
    Raises ValueError if format is invalid.
    """
    m = SEMVER_RE.match(tag)
    if not m:
        raise ValueError(f"Invalid version tag format: {tag} (expected v<major>.<minor>.<patch>)")
    return tuple(map(int, m.groups()))


def bump_version(cur: str | None, bump: str) -> str:
    """
    Given the current version and bump type, returns the next version tag.
    bump: 'major', 'minor', or 'patch'
    """
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
    Auto-detects the version bump type by diffing staged files vs. previous commit.
    - 'major': any change in public API headers (include/)
    - 'minor': any new feature in src/ (added files)
    - 'patch': everything else
    Returns: 'major', 'minor', or 'patch'
    """
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
    """
    Checks if a git tag already exists.
    Raises RuntimeError if tag exists and --force is not set.
    """
    try:
        _run(["git", "rev-parse", "-q", "--verify", f"refs/tags/{tag}"], root)
        raise RuntimeError(f"Tag already exists: {tag}")
    except subprocess.CalledProcessError:
        return  # ok


def create_and_push_tag(root: Path, tag: str, message: str, force: bool) -> None:
    """
    Creates an annotated git tag and pushes it to origin.
    If --force is set, overwrites existing tag.
    """
    if not force:
        ensure_tag_available(root, tag)
    tag_cmd = ["git", "tag", "-a", tag, "-m", message]
    if force:
        tag_cmd.insert(2, "-f")
    _run(tag_cmd, root)
    _run(["git", "push", "origin", tag] + (["-f"] if force else []), root)


def parse_args() -> argparse.Namespace:
    """
    Parses CLI arguments for release options.
    Supports mutually exclusive bump flags, explicit version, dry-run, force, and tag message.
    """
    p = argparse.ArgumentParser(description="Create a release: build → validate → tag → push")
    g = p.add_mutually_exclusive_group()
    g.add_argument("--patch", action="store_true", help="Bump patch version")
    g.add_argument("--minor", action="store_true", help="Bump minor version")
    g.add_argument("--major", action="store_true", help="Bump major version")
    g.add_argument("--version", type=str, help="Explicit version tag (vX.Y.Z)")
    p.add_argument("--dry-run", action="store_true", help="Show actions only")
    p.add_argument("--force", action="store_true", help="Overwrite existing tag if present")
    p.add_argument("--yes", "-y", action="store_true", help="Non-interactive: assume 'yes' to all prompts (CI-friendly)")
    p.add_argument("message", type=str, help="Annotated tag message")
    return p.parse_args()


def ensure_clean_worktree(root: Path):
    """
    Blocks release if there are unstaged or uncommitted changes.
    """
    status = _run(["git", "status", "--porcelain"], root)
    if status.strip():
        raise RuntimeError(
            "Release blocked: Uncommitted changes present. "
            "Please commit or stash all changes before releasing."
        )


def ensure_on_main(root: Path):
    """
    Blocks release if not on the 'main' branch.
    """
    branch = _run(["git", "rev-parse", "--abbrev-ref", "HEAD"], root)
    if branch != "main":
        raise RuntimeError(f"Release blocked: Not on main branch (current: {branch})")


def ensure_synced_with_origin(root: Path):
    """
    Blocks release if local main is behind origin/main.
    Prints instructions for the user to fix.
    """
    local = _run(["git", "rev-parse", "main"], root)
    remote = _run(["git", "rev-parse", "origin/main"], root)
    if local != remote:
        print(
            "\nERROR: Local main is behind origin/main.\n"
            "To fix:\n"
            "  git checkout main\n"
            "  git pull --rebase origin main\n"
            "Then re-run the release script.\n",
            file=sys.stderr
        )
        raise RuntimeError("Release blocked: Local main is not up-to-date with origin/main.")


def confirm_commit_message(root: Path, assume_yes: bool = False):
    """
    Before proceeding, show the user the staged files and prompt for confirmation.
    If assume_yes is True, skip prompt and continue.
    """
    diff = _run(["git", "diff", "--cached", "--name-status", "HEAD"], root)
    files = [line.split('\t')[-1] for line in diff.splitlines() if line]
    print(f"\nYou are about to commit {len(files)} file(s):")
    for f in files:
        print(f"  - {f}")
    if assume_yes:
        print("Auto-confirmed (--yes): proceeding.")
        return
    resp = input("Continue? [y/n]: ").strip().lower()
    if resp != "y":
        print("Release aborted by user.")
        sys.exit(1)


def warn_api_change_and_confirm(root: Path, assume_yes: bool = False):
    """
    If any staged file in include/ is present, warn and prompt user to continue.
    If assume_yes is True, proceed without prompting.
    """
    diff = _run(["git", "diff", "--cached", "--name-status", "HEAD"], root)
    api_files = [line.split('\t')[-1] for line in diff.splitlines() if line and line.split('\t')[-1].startswith("include/")]
    if api_files:
        print("\nWARNING: The following staged files contain public-facing API changes:")
        for f in api_files:
            print(f"  - {f}")
        print("This will result in a new Major release.")
        if assume_yes:
            print("Auto-confirmed (--yes): proceeding with major release.")
            return
        resp = input("Continue? [y/n]: ").strip().lower()
        if resp != "y":
            print("Release aborted by user.")
            sys.exit(1)


def abort_and_reset(root: Path):
    """
    Abort the release and roll back all staged changes.
    """
    print("\nCommit Aborted.")
    # Unstage everything (reset to HEAD)
    subprocess.run(["git", "reset", "--hard", "HEAD"], cwd=root)
    sys.exit(2)


def append_release_footer(tag: str, notes_path: Path):
    """
    Appends a small, light, italicized footnote to ReleaseNotes.md with instructions for checking out the release.
    """
    footer = (
        f"\n<sub><i>To check out this release locally:</i><br>"
        f"<i><code>git fetch --tags</code><br>"
        f"<code>git checkout {tag}</code></i></sub>\n"
    )
    with open(notes_path, "a") as f:
        f.write(footer)


def archive_release_notes(root: Path, tag: str):
    """
    Copies ReleaseNotes.md to docs/release_notes/ReleaseNotes-<version>.md for archival.
    """
    notes_src = root / "ReleaseNotes.md"
    archive_dir = root / "docs" / "release_notes"
    archive_dir.mkdir(parents=True, exist_ok=True)
    notes_dst = archive_dir / f"ReleaseNotes-{tag}.md"
    shutil.copy2(notes_src, notes_dst)
    print(f"Archived release notes to {notes_dst}")


def generate_changelog(root: Path, tag: str, prev_tag: str | None) -> str:
    """
    Generates a changelog section from git commits since the previous tag.
    Converts PR numbers in commit messages to clickable GitHub links.
    """
    repo_url = "https://github.com/<your-org-or-user>/<your-repo>"  # <-- update this!
    if prev_tag:
        range_spec = f"{prev_tag}..{tag}"
    else:
        range_spec = tag
    log_cmd = [
        "git", "log", "--pretty=format:- %h %s (%an)", range_spec
    ]
    try:
        log = _run(log_cmd, root)
    except Exception:
        log = ""
    # Replace (#NNN) with PR links
    import re
    def pr_link(match):
        pr_num = match.group(1)
        return f"[#{pr_num}]({repo_url}/pull/{pr_num})"
    log = re.sub(r"\(#(\d+)\)", pr_link, log)
    changelog = f"\n## {tag}\n\n### Changes since {prev_tag or 'beginning'}\n{log}\n"
    return changelog


def update_release_notes(root: Path, tag: str, prev_tag: str | None):
    """
    Appends the changelog for this release to ReleaseNotes.md and archives it.
    """
    notes_path = root / "ReleaseNotes.md"
    changelog = generate_changelog(root, tag, prev_tag)
    with open(notes_path, "a") as f:
        f.write(changelog)
    append_release_footer(tag, notes_path)
    append_dependency_versions(notes_path)
    archive_release_notes(root, tag)


def append_dependency_versions(notes_path: Path):
    """
    Appends a light bold 'Third Party' section with dependency versions to ReleaseNotes.md.
    Only lists third-party dependencies. Versions should be set dynamically or via config.
    """
    # Example: get versions from config, files, or CMake (replace with actual detection)
    googletest_version = "detect or set here"
    json_version = "detect or set here"
    cmake_version = _run(["cmake", "--version"], Path(".")).split()[2]
    clang_version = _run(["clang", "--version"], Path(".")).split()[2] if shutil.which("clang") else "not found"

    deps = [
        f"googletest {googletest_version}",
        f"json {json_version}",
        f"CMake {cmake_version}",
        f"clang compiler: {clang_version}"
    ]
    section = (
        f"\n<strong><sub>Third Party:</sub></strong>\n"
        + "\n".join(f"<strong><sub>{dep}</sub></strong>" for dep in deps)
        + "\n"
    )
    with open(notes_path, "a") as f:
        f.write(section)


def main() -> int:
    """
    Main release workflow:
    1. Build all configs via build.py
    2. Validate expected binaries
    3. Determine next version (auto or manual)
    4. Create and push git tag (unless dry-run)
    """
    args = parse_args()
    dry_run = args.dry_run
    assume_yes = getattr(args, "yes", False)
    root = repo_root()
    print("DRY-RUN: Creating release..." if dry_run else "Creating release...")

    # 0) Preconditions: ensure clean worktree / branch / sync
    ensure_clean_worktree(root)
    ensure_on_main(root)
    ensure_synced_with_origin(root)

    # 0.5) Show staged files and ask to proceed (skip if --yes)
    confirm_commit_message(root, assume_yes=assume_yes)
    warn_api_change_and_confirm(root, assume_yes=assume_yes)

    # 1) Build all and 2) validate
    build_all(root, dry_run=dry_run)
    validate_outputs(root, dry_run=dry_run)

    # 3) Resolve version
    latest = get_latest_tag(root)
    if args.version:
        if not SEMVER_RE.match(args.version):
            raise ValueError("Version must be v<major>.<minor>.<patch>")
        if latest is not None:
            latest_tuple = parse_semver(latest)
            explicit_tuple = parse_semver(args.version)
            # Only allow forward version bumps
            if explicit_tuple <= latest_tuple:
                raise ValueError(f"Explicit version {args.version} must be greater than latest {latest}")
            # Prevent major version bump > 1
            if explicit_tuple[0] > latest_tuple[0] + 1:
                raise ValueError(f"Major version can only be bumped by 1 (latest: {latest}, requested: {args.version})")
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
        next_tag = bump_version(latest, bump)

    # If API changes detected and bump is (or will be) major, warn and confirm (skip if --yes)
    warn_api_change_and_confirm(root, assume_yes=assume_yes)

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
        # Handles Ctrl+C, Command+C, or any keyboard interrupt (cross-platform)
        abort_and_reset(repo_root())
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
