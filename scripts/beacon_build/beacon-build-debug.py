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
    script_path = app_dir / "beacon-build-debug.sh"
    if not script_path.exists():
        logging.error(f"Build script not found: {script_path}")
        # Print files in the app directory for debugging
        logging.error(f"Files in {app_dir}: {list(app_dir.glob('*'))}")
        return False
    try:
        subprocess.run(["bash", str(script_path)], check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Build failed for {app_dir.name}: {e}")
        return False
    return True

def main():
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent
    log_file = (repo_root / "logs/beacon-build-debug.log").resolve()
    log_file.parent.mkdir(parents=True, exist_ok=True)
    setup_logging(log_file)

    apps = [
        repo_root / "src/apps/exchange_matching_engine",
        repo_root / "src/apps/exchange_market_data_generator",
        repo_root / "src/apps/exchange_market_data_playback",
        repo_root / "src/apps/client_algorithm",
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