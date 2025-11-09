import subprocess
import sys
from pathlib import Path
import logging
import shutil

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
    source_dir = str(app_dir)
    build_dir = app_dir / "build-release"
    # Clean step: remove previous build directory if it exists
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)
    cmake_args = [
        source_dir,
        "-DCMAKE_BUILD_TYPE=Release",
        '-DCMAKE_CXX_FLAGS=-O3 -march=native -Wall -Wextra -Wpedantic'
    ]
    try:
        subprocess.run(["cmake"] + cmake_args, cwd=str(build_dir), check=True)
        subprocess.run(["cmake", "--build", "."], cwd=str(build_dir), check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Build failed for {app_dir.name}: {e}")
        return False
    return True

def main():
    script_dir = Path(__file__).resolve().parent
    log_file = (script_dir / "../logs/beacon-build-release.log").resolve()
    log_file.parent.mkdir(parents=True, exist_ok=True)  # Ensure logs directory exists
    setup_logging(log_file)

    project_root = script_dir.parent.parent  # Go up to Beacon/
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
        logging.info("All release builds succeeded.")
        return 0
    else:
        logging.error("Some release builds failed.")
        return 1

if __name__ == "__main__":
    sys.exit(main())