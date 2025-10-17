# Progress Summary

## Refactored `build.sh`
- Centralized reusable logic for building apps.
- Made it app-agnostic and capable of building all apps (`file_generator`, `udp_replayer`, `matching_engine`) sequentially.

## Reorganized `scripts` Directory
- Created `scripts/build` for all build-related scripts.
- Created `scripts/release` for release-related scripts, including git-specific tasks and release notes generation.

## Simplified App-Specific Scripts
- Delegated logic to `build.sh` while allowing app-specific extensions if needed.

## Moved Git-Specific Scripts
- Relocated git-related scripts from `utils/git` to `scripts/release`.

## Next Steps
- Add any app-specific dependencies or configurations to their respective scripts under `scripts/build`.
- Expand `scripts/release` with additional release-related tasks as needed.

This document serves as a checkpoint to easily pick up where you left off.