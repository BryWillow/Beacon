#!/usr/bin/env python3

import subprocess
import sys
import os

def main():
    # Set the playback app root directory
    app_root = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(app_root, "build")

    # Create build directory if it doesn't exist
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Run CMake (assumes CMakeLists.txt is in playback app root)
    print("Configuring with CMake...")
    cmake_cmd = ["cmake", ".."]
    result = subprocess.run(cmake_cmd, cwd=build_dir)
    if result.returncode != 0:
        print("CMake configuration failed.")
        sys.exit(1)

    # Run make
    print("Building with make...")
    make_cmd = ["make", "-j"]
    result = subprocess.run(make_cmd, cwd=build_dir)
    if result.returncode != 0:
        print("Build failed.")
        sys.exit(1)

    print("Beacon playback build complete.")

if __name__ == "__main__":
    main()
