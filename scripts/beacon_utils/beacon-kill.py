#!/usr/bin/env python3
"""
Kill all Beacon trading system processes.

Comprehensive cleanup script that:
- Finds and terminates all Beacon processes
- Tries graceful SIGTERM first, then SIGKILL if needed
- Verifies cleanup and reports lingering processes
- Checks for bound sockets
- Offers to clean up temporary log files
"""

import glob
import os
import signal
import subprocess
import sys
import time
from pathlib import Path
from typing import List, Tuple


class ProcessInfo:
    """Information about a process to be killed."""
    def __init__(self, pid: int, name: str):
        self.pid = pid
        self.name = name
        self.killed = False
        self.failed = False


def find_processes(patterns: List[str]) -> List[ProcessInfo]:
    """
    Find all processes matching the given patterns.
    
    Args:
        patterns: List of process name patterns to search for
        
    Returns:
        List of ProcessInfo objects for found processes
    """
    processes = []
    current_pid = os.getpid()
    
    for pattern in patterns:
        try:
            result = subprocess.run(
                ["pgrep", "-f", pattern],
                capture_output=True,
                text=True
            )
            
            if result.returncode == 0 and result.stdout.strip():
                pids = result.stdout.strip().split('\n')
                
                for pid_str in pids:
                    pid = int(pid_str)
                    
                    # Skip our own process
                    if pid == current_pid:
                        continue
                    
                    # Get process name
                    try:
                        ps_result = subprocess.run(
                            ["ps", "-p", str(pid), "-o", "comm="],
                            capture_output=True,
                            text=True
                        )
                        
                        if ps_result.returncode == 0:
                            name = ps_result.stdout.strip()
                            if name:
                                processes.append(ProcessInfo(pid, name))
                    except Exception:
                        pass
                        
        except Exception as e:
            print(f"Warning: Error searching for {pattern}: {e}")
    
    return processes


def kill_process(proc: ProcessInfo) -> bool:
    """
    Kill a single process, trying SIGTERM first, then SIGKILL.
    
    Args:
        proc: ProcessInfo object
        
    Returns:
        True if successfully killed, False otherwise
    """
    try:
        # Try graceful termination first (SIGTERM)
        os.kill(proc.pid, signal.SIGTERM)
        time.sleep(0.1)
        
        # Check if process still exists
        try:
            os.kill(proc.pid, 0)  # Signal 0 just checks existence
            # Process still running, use SIGKILL
            os.kill(proc.pid, signal.SIGKILL)
            print(f"  [KILLED] PID {proc.pid}: {proc.name} (forced)")
        except OSError:
            # Process terminated gracefully
            print(f"  [KILLED] PID {proc.pid}: {proc.name}")
        
        return True
        
    except ProcessLookupError:
        # Process already dead
        print(f"  [GONE]   PID {proc.pid}: {proc.name} (already terminated)")
        return True
    except PermissionError:
        print(f"  [FAILED] PID {proc.pid}: {proc.name} (permission denied)")
        return False
    except Exception as e:
        print(f"  [FAILED] PID {proc.pid}: {proc.name} ({e})")
        return False


def check_sockets(ports: List[int]) -> List[str]:
    """
    Check for processes bound to specific ports.
    
    Args:
        ports: List of port numbers to check
        
    Returns:
        List of lines describing bound sockets
    """
    try:
        port_args = []
        for port in ports:
            port_args.extend(["-i", f":{port}"])
        
        result = subprocess.run(
            ["lsof"] + port_args,
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0 and result.stdout.strip():
            lines = result.stdout.strip().split('\n')
            # Skip header line
            return lines[1:] if len(lines) > 1 else []
        
    except Exception:
        pass
    
    return []


def check_log_files() -> List[Path]:
    """
    Find temporary Beacon log files.
    
    Returns:
        List of Path objects for log files
    """
    log_files = []
    try:
        log_files = list(Path("/tmp").glob("beacon_*.log"))
    except Exception:
        pass
    
    return log_files


def main():
    """Kill all Beacon processes with comprehensive cleanup."""
    print("=" * 70)
    print("  BEACON TRADING SYSTEM - PROCESS CLEANUP")
    print("=" * 70)
    print()
    
    # Define process patterns to search for
    patterns = [
        "exchange_matching_engine",
        "exchange_market_data_playback",
        "exchange_market_data_generator",
        "client_algorithm",
        "client_algo",
        "algo_twap",
        "algo_template",
        "algo_vwap",
        "udp_listener.py",
        "test_pillar",
        "test_cme",
    ]
    
    # Find all processes
    print("Scanning for running processes...")
    print()
    
    processes = find_processes(patterns)
    
    if not processes:
        print("✓ No Beacon processes found running")
        print()
        return 0
    
    # Display found processes
    for proc in processes:
        print(f"  [FOUND]  PID {proc.pid}: {proc.name}")
    
    print()
    print(f"Found {len(processes)} process(es) to kill")
    print()
    
    # Kill all processes
    print("Attempting to kill processes...")
    print()
    
    killed_count = 0
    failed_count = 0
    
    for proc in processes:
        if kill_process(proc):
            killed_count += 1
            proc.killed = True
        else:
            failed_count += 1
            proc.failed = True
    
    # Summary
    print()
    print("=" * 70)
    print("  CLEANUP SUMMARY")
    print("=" * 70)
    print(f"  Found:    {len(processes)}")
    print(f"  Killed:   {killed_count}")
    print(f"  Failed:   {failed_count}")
    print("=" * 70)
    print()
    
    # Verify cleanup
    print("Verifying cleanup...")
    remaining = find_processes(patterns)
    
    if remaining:
        print(f"✗ {len(remaining)} process(es) still running:")
        for proc in remaining:
            print(f"  [STILL RUNNING] PID {proc.pid}: {proc.name}")
        print()
    else:
        print("✓ All processes successfully terminated")
        print()
    
    # Check for lingering sockets
    print("Checking for lingering sockets...")
    sockets = check_sockets([9000, 12345])
    
    if sockets:
        print("Found bound sockets:")
        for socket_line in sockets:
            print(f"  {socket_line}")
        print("(Will be cleaned up when processes exit)")
    else:
        print("✓ No sockets bound to ports 9000 or 12345")
    
    print()
    
    # Check for log files
    print("Checking for temporary log files...")
    log_files = check_log_files()
    
    if log_files:
        print("Found log files:")
        for log_file in log_files:
            size = log_file.stat().st_size
            size_kb = size / 1024
            print(f"  {log_file} ({size_kb:.1f} KB)")
        
        print()
        try:
            response = input("Delete log files? (y/n): ").strip().lower()
            if response in ('y', 'yes'):
                for log_file in log_files:
                    try:
                        log_file.unlink()
                    except Exception as e:
                        print(f"  Warning: Could not delete {log_file}: {e}")
                print("✓ Log files deleted")
            else:
                print("Log files kept")
        except (KeyboardInterrupt, EOFError):
            print("\nLog files kept")
    else:
        print("✓ No temporary log files found")
    
    print()
    
    # Exit code based on remaining processes
    if remaining:
        return 1
    else:
        return 0


if __name__ == "__main__":
    sys.exit(main())
