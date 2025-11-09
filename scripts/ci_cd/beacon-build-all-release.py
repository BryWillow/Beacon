import subprocess
from pathlib import Path

def build_app(app_name):
    repo_root = Path(__file__).resolve().parents[2]
    script_path = repo_root / f"src/apps/{app_name}/beacon-build-release.sh"
    print(f"[DEBUG] Building {app_name} using {script_path}")
    if not script_path.exists():
        print(f"[ERROR] Build script not found: {script_path}")
        return False
    try:
        subprocess.run(["bash", str(script_path)], check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Build failed for {app_name}: {e}")
        return False

if __name__ == "__main__":
    apps = [
        "exchange_matching_engine",
        "exchange_market_data_generator",
        "exchange_market_data_playback",
        "client_algorithm",
    ]
    all_success = True
    for app in apps:
        if not build_app(app):
            all_success = False
    if all_success:
        print("[CI/CD] All release builds complete!")
    else:
        print("[CI/CD] Some release builds failed.")
