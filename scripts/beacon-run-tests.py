#!/usr/bin/env python3
"""
Build and run all GoogleTest suites for the Beacon trading system.

This script builds the test suite using CMake and runs all tests with
detailed output using ctest.
"""

import subprocess
import sys
from pathlib import Path
from typing import Optional


def run_command(
    cmd: list[str], 
    cwd: Optional[Path] = None,
    description: str = ""
) -> bool:
    """
    Run a command and return success/failure.
    
    Args:
        cmd: Command and arguments to execute
        cwd: Working directory (None = current directory)
        description: Human-readable description of what's running
        
    Returns:
        True if command succeeded (exit code 0), False otherwise
    """
    if description:
        print(f"\n{description}")
    
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            capture_output=False,
            text=True
        )
        return result.returncode == 0
    except Exception as e:
        print(f"✗ Error: {e}")
        return False


def main():
    """Build and run all test suites."""
    # Get project root (one level up from scripts/)
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    build_dir = project_root / "build"
    
    print("=" * 70)
    print("  BEACON TRADING SYSTEM - TEST SUITE")
    print("=" * 70)
    print(f"\nProject root: {project_root}")
    print(f"Build directory: {build_dir}")
    
    # Create build directory if it doesn't exist
    build_dir.mkdir(exist_ok=True)
    
    # Step 1: Configure with CMake
    print("\n" + "=" * 70)
    print("  STEP 1: Configuring CMake")
    print("=" * 70)
    
    if not run_command(
        ["cmake", ".."],
        cwd=build_dir,
        description="Running CMake configuration..."
    ):
        print("\n✗ CMake configuration failed")
        return 1
    
    print("✓ CMake configuration successful")
    
    # Step 2: Build tests
    print("\n" + "=" * 70)
    print("  STEP 2: Building Test Suite")
    print("=" * 70)
    
    if not run_command(
        ["make", "-j8"],
        cwd=build_dir,
        description="Building all test executables..."
    ):
        print("\n✗ Build failed")
        return 1
    
    print("✓ Test suite built successfully")
    
    # Step 3: Run tests
    print("\n" + "=" * 70)
    print("  STEP 3: Running All Tests")
    print("=" * 70)
    print()
    
    if not run_command(
        ["ctest", "--output-on-failure", "--verbose"],
        cwd=build_dir
    ):
        print("\n" + "=" * 70)
        print("  TEST RESULTS: FAILED")
        print("=" * 70)
        print("\n✗ Some tests failed. Check output above for details.")
        return 1
    
    # Success!
    print("\n" + "=" * 70)
    print("  TEST RESULTS: ALL PASSED ✓")
    print("=" * 70)
    print("\n✓ All tests passed successfully!")
    print("\nTest coverage:")
    print("  • Core components (ringbuffer, threading, latency)")
    print("  • Protocol implementations (ITCH, Pillar, MDP, OUCH)")
    print("  • Matching engine")
    print("  • Market data generation")
    print("  • Client algorithms")
    print("  • Integration tests")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
