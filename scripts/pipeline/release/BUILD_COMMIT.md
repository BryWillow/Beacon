# Build & Commit Process

This document describes the **build and commit pipeline** for the NSDQ ITCH Beacon project. It is designed for **single-developer safety, reproducibility, and strict correctness**.

---

## 1. Overview

We maintain two scripts:

1. `./build.sh` – automated builds, versioning, stubs, unit tests, optional linters.
2. `./scripts/git_commit.sh` – staged-only commits, bullet-point messages, forced confirmation, success/error logs.

This pipeline is **local and manual**, forming the foundation for future CI/CD.

---

## 2. Key Rules

- Never commit `src/*.cpp` or `*.h` directly from GitHub.
- Documentation changes are allowed (README, PDFs, etc.).
- Version increments:
  - Build number auto-increments on each build
  - Major/Minor/Patch only changed via `-v` in `build.sh all` mode
- Commit messages include full context to recreate build.

---

## 3. Workflow

1. `./build.sh debug|release|all` → builds, runs tests, writes release notes.
2. Stage files to commit: `git add .`
3. `./scripts/git_commit.sh` → generates commit message, review, type `yes` to commit.

---

## 4. Logs

- Commit messages and results are saved in `scripts/git_commit_logs/`
- Logs are timestamped for historical reference.

---

## 5. Example Commands

```bash
./build.sh debug
./build.sh release
./build.sh all -t:false -v:1.2.3 -c:false
./scripts/git_commit.sh
