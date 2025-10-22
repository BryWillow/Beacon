#!/usr/bin/env python3
"""
DEPRECATED: use ./build.py instead.
This script is kept as a thin delegator to avoid duplication and preserve existing workflows.
"""

import sys
import subprocess
from pathlib import Path


def main() -> int:
    repo_root = Path(__file__).resolve().parent
    build_script = repo_root / "build.py"
    if not build_script.exists():
        print("ERROR: Missing build.py at repo root.", file=sys.stderr)
        return 1

    # Minimal notice; keep output clean
    print("Note: build_beacon.py is deprecated. Use ./build.py.", file=sys.stderr)

    # Forward all args to build.py (debug by default handled by build.py)
    cmd = [sys.executable, str(build_script), *sys.argv[1:]]
    try:
        subprocess.run(cmd, check=True, cwd=repo_root)
        return 0
    except subprocess.CalledProcessError as e:
        return e.returncode


if __name__ == "__main__":
    sys.exit(main())