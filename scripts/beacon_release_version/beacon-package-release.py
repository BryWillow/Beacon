import os
import shutil
import sys

def copytree(src, dst, exclude_files=None):
    exclude_files = exclude_files or []
    for root, dirs, files in os.walk(src):
        rel_path = os.path.relpath(root, src)
        target_dir = os.path.join(dst, rel_path)
        os.makedirs(target_dir, exist_ok=True)
        for file in files:
            if file in exclude_files:
                continue
            src_file = os.path.join(root, file)
            dst_file = os.path.join(target_dir, file)
            shutil.copy2(src_file, dst_file)

def main():
    if len(sys.argv) != 2:
        print("Usage: python beacon-package-release.py <version>")
        sys.exit(1)
    version = sys.argv[1]
    release_dir = f"v{version}"
    if os.path.exists(release_dir):
        print(f"Release directory {release_dir} already exists. Aborting.")
        sys.exit(1)
    os.makedirs(release_dir)

    # Copy scripts, excluding test/release scripts
    scripts_src = "scripts"
    scripts_dst = os.path.join(release_dir, "scripts")
    exclude_scripts = ["beacon-run-tests.py", "beacon-create-release.py", "beacon-package-release.py"]
    copytree(scripts_src, scripts_dst, exclude_files=exclude_scripts)

    # Copy binaries to bin/
    bin_src = "bin"
    bin_dst = os.path.join(release_dir, "bin")
    if os.path.exists(bin_src):
        shutil.copytree(bin_src, bin_dst)
    else:
        os.makedirs(bin_dst)

    print(f"Release package created at {release_dir}")

if __name__ == "__main__":
    main()
