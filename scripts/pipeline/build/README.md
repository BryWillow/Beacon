# Build Scripts

This directory contains scripts for building individual components of the Beacon project. Each script is responsible for building a specific app or module.

## Scripts
- **build_file_generator.sh**: Builds the `file_generator` app.
- **build_udp_replayer.sh**: Builds the `udp_replayer` app.
- **build_matching_engine.sh**: Builds the `matching_engine` app.

## Usage
These scripts are typically invoked by the top-level `build.sh` script, but they can also be run individually if needed. For example:
```bash
./build_file_generator.sh debug
```