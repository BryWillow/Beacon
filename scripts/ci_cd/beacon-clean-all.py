import shutil
from pathlib import Path

def clean(paths):
    for p in paths:
        p = Path(p)
        try:
            if p.exists():
                if p.is_dir():
                    shutil.rmtree(p)
                else:
                    p.unlink()
                print(f"Removed: {p}")
            else:
                print(f"Skipped (not found): {p}")
        except Exception as e:
            print(f"Error removing {p}: {e}")

if __name__ == "__main__":
    repo_root = Path(__file__).resolve().parents[2]
    print(f"[DEBUG] Repo root: {repo_root}")
    clean([
        repo_root / "src/apps/exchange_matching_engine/build-debug",
        repo_root / "src/apps/exchange_matching_engine/build-release",
        repo_root / "src/apps/exchange_market_data_generator/build-debug",
        repo_root / "src/apps/exchange_market_data_generator/build-release",
        repo_root / "src/apps/exchange_market_data_playback/build-debug",
        repo_root / "src/apps/exchange_market_data_playback/build-release",
        repo_root / "src/apps/client_algorithm/build-debug",
        repo_root / "src/apps/client_algorithm/build-release",
        repo_root / "logs",
        repo_root / "scripts/logs",
    ])
    print("[CI/CD] Clean complete!")
