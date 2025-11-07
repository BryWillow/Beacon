from pathlib import Path
import sys
import subprocess
import logging
import shutil

# TIP: This script copies the debug binary to bin/debug after building.
# This makes orchestration, packaging, and CI/CD easier, while keeping the binary available in the build directory for local testing and debugging.
# =============================================================================
# Project:      Beacon
# Application:  exchange_market_data_generator
# Purpose:      Debug build script for the market data generator
# Author:       Bryan Camp
# =============================================================================

def setup_logging(log_file):
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(levelname)s: %(message)s',
        handlers=[
            logging.StreamHandler(sys.stdout),
            logging.FileHandler(log_file, mode='a')
        ]
    )

def build_app(app_dir: Path):
    build_dir = app_dir / "build-debug"
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)
    cmake_args = [
        str(app_dir),
        "-DCMAKE_BUILD_TYPE=Debug",
        '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -Wall -Wextra -Wpedantic -O0'
    ]
    try:
        # Configure
        subprocess.run(["cmake"] + cmake_args, cwd=str(build_dir), check=True)
        # Build
        subprocess.run(["cmake", "--build", "."], cwd=str(build_dir), check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Build failed for {app_dir.name}: {e}")
        return False
    return True

def main():
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent.parent  # <-- Only go up two levels to Beacon/
    log_file = script_dir / "../logs/beacon-build-debug.log"
    setup_logging(log_file)

    apps = [
        project_root / "src/apps/exchange_matching_engine",
        project_root / "src/apps/exchange_market_data_generator",
        project_root / "src/apps/exchange_market_data_playback",
        project_root / "src/apps/client_algorithm",
    ]
    all_success = True
    for app in apps:
        success = build_app(app)
        if not success:
            all_success = False

    if all_success:
        logging.info("All debug builds succeeded.")
        return 0
    else:
        logging.error("Some debug builds failed.")
        return 1

if __name__ == "__main__":
    sys.exit(main())