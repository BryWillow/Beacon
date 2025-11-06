#!/usr/bin/env python3
"""
Build all components of the Beacon trading system.
"""

import subprocess
import sys
from pathlib import Path
from typing import List, Tuple

def run_build(component_name: str, build_script: Path) -> bool:
    """
    Run a build script and return success/failure.
    
    Args:
        component_name: Human-readable name of component
        build_script: Path to build.sh script
        
    Returns:
        True if build succeeded, False otherwise
    """
    print(f"\n{'=' * 70}")
    print(f"  Building: {component_name}")
    print(f"{'=' * 70}")
    
    if not build_script.exists():
        print(f"✗ Build script not found: {build_script}")
        return False
    
    try:
        # Run the build script in its directory
        result = subprocess.run(
            [str(build_script)],
            cwd=build_script.parent,
            capture_output=False,
            text=True
        )
        
        if result.returncode == 0:
            print(f"✓ {component_name} built successfully")
            return True
        else:
            print(f"✗ {component_name} build failed (exit code: {result.returncode})")
            return False
            
    except Exception as e:
        print(f"✗ Error building {component_name}: {e}")
        return False


def main():
    """Build all components."""
    # Get project root (one level up from scripts/)
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    
    print("=" * 70)
    print("  BEACON TRADING SYSTEM - BUILD ALL COMPONENTS")
    print("=" * 70)
    print(f"\nProject root: {project_root}")
    
    # Define components to build (in dependency order)
    components = [
        ("Exchange Matching Engine", project_root / "src/apps/exchange_matching_engine/build.sh"),
        ("Market Data Generator", project_root / "src/apps/exchange_market_data_generator/build.sh"),
        ("Market Data Playback", project_root / "src/apps/exchange_market_data_playback/build.sh"),
        ("Client Algorithms", project_root / "src/apps/client_algorithm/build.sh"),
    ]
    
    # Track results
    results: List[Tuple[str, bool]] = []
    
    # Build each component
    for name, build_script in components:
        success = run_build(name, build_script)
        results.append((name, success))
    
    # Summary
    print()
    print("=" * 70)
    print("  BUILD SUMMARY")
    print("=" * 70)
    
    all_success = True
    for name, success in results:
        status = "✓" if success else "✗"
        print(f"  {status} {name}")
        if not success:
            all_success = False
    
    print("=" * 70)
    
    if all_success:
        print("\n✓ All components built successfully!")
        print("\nNext steps:")
        print("  1. Generate market data: cd src/apps/exchange_market_data_generator && ./run.sh")
        print("  2. Run the system: python3 scripts/run_system.py 30")
        return 0
    else:
        print("\n✗ Some builds failed. Check output above for details.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
