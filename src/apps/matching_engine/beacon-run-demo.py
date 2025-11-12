import subprocess
import time
import os
from pathlib import Path

def run_process(cmd, cwd=None):
    print(f"[RUN] {' '.join(cmd)}")
    return subprocess.Popen(cmd, cwd=cwd)

def wait_for_handshake(process_name, timeout=10):
    print(f"[WAIT] Waiting for handshake from {process_name}...")
    # TODO: Implement actual handshake detection (e.g., check log file, port, or stdout)
    time.sleep(2)  # Simulate handshake wait

def main():
    repo_root = Path(__file__).resolve().parents[3]
    engine_bin = repo_root / "src/apps/exchange_matching_engine/bin/debug/exchange_matching_engine"
    algo_bin = repo_root / "src/apps/client_algorithm/bin/debug/client_algorithm"
    md_playback_bin = repo_root / "src/apps/exchange_market_data_playback/bin/debug/exchange_market_data_playback"

    # 1. Start matching engine
    engine_proc = run_process([str(engine_bin), "--demo"])
    wait_for_handshake("matching engine")

    # 2. Start algo
    algo_proc = run_process([str(algo_bin), "--demo"])
    wait_for_handshake("algo")

    # 3. Start market data playback
    md_proc = run_process([str(md_playback_bin), "--demo"])
    wait_for_handshake("market data playback")

    print("[INFO] All components started. Demo running.")
    try:
        # Wait for user to terminate or for any process to exit
        engine_proc.wait()
        algo_proc.wait()
        md_proc.wait()
    except KeyboardInterrupt:
        print("[INFO] Terminating demo processes...")
        engine_proc.terminate()
        algo_proc.terminate()
        md_proc.terminate()

if __name__ == "__main__":
    main()
