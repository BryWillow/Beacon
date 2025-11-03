#!/bin/bash
set -e

BUILD_DIR="build"
cd "$BUILD_DIR"
cp ../sample_config.json .

# Run the md_generator with the sample configuration.
# Binary ITCH will be written to output.itch in the
./md_generator sample_config.json output.itch
