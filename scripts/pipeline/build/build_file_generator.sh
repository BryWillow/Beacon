#!/usr/bin/env bash
set -e

# Delegate to the main build script
"$(git rev-parse --show-toplevel)/build.sh" "$@"
