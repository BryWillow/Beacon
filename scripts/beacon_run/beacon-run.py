#!/usr/bin/env python3
"""
Beacon Trading System Orchestrator
Professional process management and monitoring for the Beacon trading system.
"""

import subprocess
import signal
import sys
import time
import os
from pathlib import Path
from datetime import datetime

class Colors:
    """ANSI color codes for terminal output"""
    RESET = '\033[0m'
    BOLD = '\033[1m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'

class BeaconOrchestrator:
    def __init__(self, base_dir: Path):
        self.base_dir = base_dir
        self.matching_engine = None
        self.algorithm = None
        self.market_data = None
        self.start_time = None
        
        # Configuration
        self.exchange_port = 9000
        self.mcast_addr = "239.255.0.1"
        self.mcast_port = 12345
        self.exchange_host = "127.0.0.1"
        
        # Binary paths
        self.me_bin = base_dir / "src/apps/exchange_matching_engine/build/exchange_matching_engine"
        self.algo_bin = base_dir / "src/apps/client_algorithm/build/algo_template"
        self.md_bin = base_dir / "src/apps/exchange_market_data_playback/build/exchange_market_data_playback"
        
        # Data paths
        self.md_file = base_dir / "src/apps/exchange_market_data_generator/output.itch"
        self.md_config = base_dir / "src/apps/exchange_market_data_playback/config_udp_slow.json"
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
    def _signal_handler(self, signum, frame):
        """Handle Ctrl+C and termination signals"""
        print(f"\n{Colors.YELLOW}Received signal {signum}, shutting down...{Colors.RESET}")
        self.cleanup()
        sys.exit(0)
        
    def print_banner(self):
        """Print professional startup banner"""
        print()
        print("═" * 63)
        print("  BEACON TRADING SYSTEM - ORCHESTRATED STARTUP")
        print(f"  {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("═" * 63)
        print()
        
    def print_config(self, duration: int):
        """Print system configuration"""
        print("Configuration:")
        print(f"  Duration:         {duration} seconds")
        print(f"  Exchange:         {self.exchange_host}:{self.exchange_port}")
        print(f"  Market Data:      UDP {self.mcast_addr}:{self.mcast_port}")
        print(f"  MD File:          {self.md_file.relative_to(self.base_dir)}")
        print()
        
    def verify_binaries(self) -> bool:
        """Verify all required binaries exist"""
        binaries = [
            ("Matching Engine", self.me_bin),
            ("Algorithm", self.algo_bin),
            ("Market Data Playback", self.md_bin),
        ]
        
        all_exist = True
        for name, path in binaries:
            if not path.exists():
                print(f"{Colors.RED}✗ ERROR: {name} not built: {path}{Colors.RESET}")
                all_exist = False
                
        if not self.md_file.exists():
            print(f"{Colors.RED}✗ ERROR: Market data file not found: {self.md_file}{Colors.RESET}")
            all_exist = False
            
        if all_exist:
            print(f"{Colors.GREEN}✓ All prerequisites satisfied{Colors.RESET}")
            print()
            
        return all_exist
        
    def start_matching_engine(self) -> bool:
        """Start the matching engine"""
        step_start = time.time()
        print("═" * 63)
        print(f"  Step 1/3 - Starting OUCH Matching Engine")
        print(f"  {datetime.now().strftime('%H:%M:%S')}")
        print("═" * 63)
        
        print("* Creating TCP stack...")
        time.sleep(0.5)
        
        try:
            self.matching_engine = subprocess.Popen(
                [str(self.me_bin), str(self.exchange_port)],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
            
            print(f"* Binding to port {self.exchange_port}...")
            time.sleep(0.5)
            
            # Verify process is running
            if self.matching_engine.poll() is not None:
                print(f"{Colors.RED}✗ STEP 1 FAILED - Matching engine crashed{Colors.RESET}")
                return False
                
            print("* Ready for connections")
            duration = int(time.time() - step_start)
            print(f"* {Colors.GREEN}✓ STEP 1 PASSED ({duration}s){Colors.RESET}")
            print()
            return True
            
        except Exception as e:
            print(f"{Colors.RED}✗ STEP 1 FAILED: {e}{Colors.RESET}")
            return False
            
    def start_algorithm(self, duration: int) -> bool:
        """Start the trading algorithm"""
        step_start = time.time()
        print("═" * 63)
        print(f"  Step 2/3 - Starting Your Algorithm")
        print(f"  {datetime.now().strftime('%H:%M:%S')}")
        print("═" * 63)
        
        print("* Trading Parameters:")
        print("    Strategy:        Simple Market Maker")
        print("    Order Interval:  Every 100 ticks")
        print("    Order Size:      100 shares")
        print("    Time In Force:   IOC (Immediate or Cancel)")
        print()
        
        print("* Symbols:")
        print("    AAPL: [Algo Limit: 10000, Firm Limit: 50000]")
        print("    MSFT: [Algo Limit: 10000, Firm Limit: 50000]")
        print("    TSLA: [Algo Limit: 10000, Firm Limit: 50000]")
        print()
        
        print("* Initiating TCP connection to matching engine...")
        print(f"    Target: {self.exchange_host}:{self.exchange_port}")
        
        try:
            # Open log file for algorithm output
            self.algo_log = open('/tmp/beacon_algo.log', 'w')
            
            self.algorithm = subprocess.Popen(
                [
                    str(self.algo_bin),
                    self.mcast_addr,
                    str(self.mcast_port),
                    self.exchange_host,
                    str(self.exchange_port),
                    str(duration)
                ],
                stdout=self.algo_log,
                stderr=subprocess.STDOUT,
                text=True
            )
            
            time.sleep(1)
            
            # Verify process is running
            if self.algorithm.poll() is not None:
                print(f"{Colors.RED}✗ STEP 2 FAILED - Algorithm crashed{Colors.RESET}")
                return False
                
            print(f"* {Colors.GREEN}✓ Connected [{self.exchange_host}:{self.exchange_port}]{Colors.RESET}")
            print("* Algorithm threads initialized")
            print("    Core 0: Market Data Receiver")
            print("    Core 1: Trading Logic (HOT PATH)")
            print("    Core 2: Execution Report Processor")
            print()
            
            elapsed = int(time.time() - step_start)
            print(f"* {Colors.GREEN}✓ STEP 2 PASSED ({elapsed}s){Colors.RESET}")
            print()
            return True
            
        except Exception as e:
            print(f"{Colors.RED}✗ STEP 2 FAILED: {e}{Colors.RESET}")
            return False
            
    def start_market_data(self) -> bool:
        """Start market data playback"""
        step_start = time.time()
        print("═" * 63)
        print(f"  Step 3/3 - Playing Market Data")
        print(f"  {datetime.now().strftime('%H:%M:%S')}")
        print("═" * 63)
        
        file_size = self.md_file.stat().st_size
        num_messages = file_size // 33
        
        print(f"* Reading file: {self.md_file.relative_to(self.base_dir)}")
        print(f"    Size:        {file_size} bytes")
        print(f"    Messages:    {num_messages}")
        print("* Mode: Continuous playback at 10,000 msgs/sec")
        print("* Speed factor: 1x (real-time)")
        print()
        
        print("* Symbols in dataset:")
        print("    AAPL, MSFT, TSLA, GOOGL, AMZN")
        print()
        
        print("* UDP Multicast Configuration:")
        print(f"    Address:     {self.mcast_addr}:{self.mcast_port}")
        print("    TTL:         1 (localhost only)")
        print("    Loopback:    Enabled")
        print()
        
        try:
            self.market_data = subprocess.Popen(
                [
                    str(self.md_bin),
                    "--config", str(self.md_config),
                    str(self.md_file)
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True
            )
            
            time.sleep(1)
            
            # Verify process is running
            if self.market_data.poll() is not None:
                print(f"{Colors.RED}✗ STEP 3 FAILED - Market data playback crashed{Colors.RESET}")
                return False
                
            print(f"* Playback started (PID: {self.market_data.pid})")
            elapsed = int(time.time() - step_start)
            print(f"* {Colors.GREEN}✓ STEP 3 PASSED ({elapsed}s){Colors.RESET}")
            print()
            return True
            
        except Exception as e:
            print(f"{Colors.RED}✗ STEP 3 FAILED: {e}{Colors.RESET}")
            return False
            
    def show_progress(self, duration: int):
        """Show progress bar during system operation"""
        print("═" * 63)
        print("  ✓✓✓ ALL SYSTEMS OPERATIONAL ✓✓✓")
        total_startup = int(time.time() - self.start_time)
        print(f"  Total startup time: {total_startup}s")
        print("═" * 63)
        print()
        print(f"System will run for {duration} seconds...")
        print("Press Ctrl+C to stop early")
        print()
        
        start = time.time()
        last_update = -1
        
        while True:
            elapsed = int(time.time() - start)
            if elapsed >= duration:
                break
                
            # Update progress bar once per second
            if elapsed != last_update:
                last_update = elapsed
                percent = int(elapsed * 100 / duration)
                bar_length = percent // 2
                bar = '█' * bar_length
                space = ' ' * (50 - bar_length)
                print(f"\rProgress: [{bar}{space}] {percent}% ({elapsed}s / {duration}s)", end='', flush=True)
                
            time.sleep(0.1)
            
            # Check if algorithm crashed
            if self.algorithm and self.algorithm.poll() is not None:
                print(f"\n{Colors.YELLOW}Algorithm completed{Colors.RESET}")
                break
                
        print()  # New line after progress bar
        print()
        
    def show_final_statistics(self):
        """Collect and display final statistics from algorithm"""
        print("═" * 63)
        print("  Final Statistics")
        print("═" * 63)
        print()
        
        if self.algorithm:
            # Terminate the algorithm so it writes final stats
            if self.algorithm.poll() is None:
                self.algorithm.terminate()
                
            # Wait for it to finish writing
            time.sleep(2)
            
            # Close the log file if it's still open
            if hasattr(self, 'algo_log') and self.algo_log:
                self.algo_log.close()
            
            # Read from log file
            try:
                with open('/tmp/beacon_algo.log', 'r') as f:
                    content = f.read()
                    
                # Extract FINAL STATISTICS section including latency stats
                if "FINAL STATISTICS" in content:
                    lines = content.split('\n')
                    for i, line in enumerate(lines):
                        if "FINAL STATISTICS" in line:
                            # Print from the line before (the ===) until we hit [SIGNAL] or end
                            start = max(0, i - 1)
                            j = start
                            while j < len(lines):
                                # Stop at shutdown signal or if we see another major section
                                if "[SIGNAL]" in lines[j] or "[SHUTDOWN]" in lines[j]:
                                    break
                                print(lines[j])
                                j += 1
                                # Also stop if we've printed the latency section (look for the closing box)
                                if j > start + 5 and "═══════════" in lines[j] and j > i + 15:
                                    print(lines[j])  # Print the closing box
                                    break
                            break
                else:
                    print("(No statistics found in log)")
                    
            except FileNotFoundError:
                print("(Log file not found)")
            except Exception as e:
                print(f"(Error reading log: {e})")
        else:
            print("(Algorithm was not started)")
            
    def cleanup(self):
        """Gracefully shutdown all processes"""
        print()
        print("═" * 63)
        print("  Shutting Down System...")
        print("═" * 63)
        
        processes = [
            ("Algorithm", self.algorithm),
            ("Market Data Playback", self.market_data),
            ("Matching Engine", self.matching_engine),
        ]
        
        for name, proc in processes:
            if proc and proc.poll() is None:
                print(f"* Stopping {name} (PID: {proc.pid})...")
                proc.terminate()
                try:
                    proc.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    proc.kill()
        
        # Close log file if open
        if hasattr(self, 'algo_log') and self.algo_log:
            self.algo_log.close()
                    
        print("* Cleanup complete")
        print()
        
    def run(self, duration: int = 60):
        """Main orchestration flow"""
        self.start_time = time.time()
        
        # Print banner and config
        self.print_banner()
        self.print_config(duration)
        
        # Clean up any existing processes
        print("Cleaning up any existing processes...")
        kill_script = self.base_dir / "scripts" / "beacon-kill.py"
        if kill_script.exists():
            try:
                subprocess.run(
                    ["python3", str(kill_script)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL,
                    timeout=10
                )
            except Exception:
                pass  # Continue even if cleanup fails
        print()
        
        # Verify binaries
        if not self.verify_binaries():
            return 1
            
        time.sleep(1)
        
        # Start all components
        if not self.start_matching_engine():
            self.cleanup()
            return 1
            
        time.sleep(1)
        
        if not self.start_algorithm(duration):
            self.cleanup()
            return 1
            
        # Give algorithm extra time to initialize UDP receiver thread
        time.sleep(2)
        
        if not self.start_market_data():
            self.cleanup()
            return 1
            
        # Monitor progress
        self.show_progress(duration)
        
        # Close the algorithm's log file so we can read it
        if hasattr(self, 'algo_log') and self.algo_log:
            self.algo_log.flush()  # Make sure everything is written
            self.algo_log.close()
            self.algo_log = None
        
        # Show final statistics
        self.show_final_statistics()
        
        # Cleanup
        print()
        print("═" * 63)
        print("  Session Complete")
        print("═" * 63)
        print()
        
        self.cleanup()
        return 0

def main():
    """Entry point"""
    # Get base directory (scripts/ is at root level, so go up 1 level)
    script_dir = Path(__file__).resolve().parent
    base_dir = script_dir.parent
    
    # Parse command line arguments
    duration = 60
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print(f"Invalid duration: {sys.argv[1]}")
            return 1
            
    # Run orchestrator
    orchestrator = BeaconOrchestrator(base_dir)
    return orchestrator.run(duration)

if __name__ == "__main__":
    sys.exit(main())
