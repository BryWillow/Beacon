#!/usr/bin/env python3
"""
Kill all Beacon trading system processes.
Stops matching engine, market data playback, and algorithm processes.
"""

import subprocess
import sys
import time

def kill_processes(process_names):
    """
    Kill processes by name using pkill.
    
    Args:
        process_names: List of process name patterns to kill
        
    Returns:
        Number of processes killed
    """
    killed_count = 0
    
    for process_name in process_names:
        try:
            # First check if any processes exist
            result = subprocess.run(
                ["pgrep", "-f", process_name],
                capture_output=True,
                text=True
            )
            
            if result.returncode == 0 and result.stdout.strip():
                # Processes found, kill them
                pids = result.stdout.strip().split('\n')
                print(f"  Stopping {process_name}... (PIDs: {', '.join(pids)})")
                
                subprocess.run(
                    ["pkill", "-f", process_name],
                    capture_output=True
                )
                killed_count += len(pids)
                time.sleep(0.2)  # Give processes time to exit
            
        except Exception as e:
            print(f"  Warning: Error stopping {process_name}: {e}")
    
    return killed_count


def main():
    """Kill all Beacon processes."""
    print("=" * 70)
    print("  BEACON TRADING SYSTEM - STOP ALL PROCESSES")
    print("=" * 70)
    print()
    
    # Define process patterns to kill (in reverse startup order)
    processes = [
        "exchange_market_data_playback",
        "exchange_matching_engine",
        "algo_template",
        "client_algo",
        "algo_twap",
        "client_algorithm",
    ]
    
    killed = kill_processes(processes)
    
    print()
    print("=" * 70)
    
    if killed > 0:
        print(f"✓ Stopped {killed} process(es)")
    else:
        print("✓ No Beacon processes found running")
    
    print("=" * 70)
    return 0


if __name__ == "__main__":
    sys.exit(main())
