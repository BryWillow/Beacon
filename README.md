# The Beacon Project

## Why Beacon?
The word "Beacon" represents a guiding light—a source of hope for solving tough challenges. This project serves as a platform to explore, innovate, and measure progress in the HFT landscape.

## Project Overview
The Beacon project is an ecosystem of modular C++ applications, each designed with a single purpose but working seamlessly together. From ingesting market data at bursty, configurable rates to strategy testing, risk management, order execution, and reporting, the system is built to:
- Efficiently measure latency at every stage.
- Offer a pluggable, highly testable architecture.
- Provide optimized implementations by default.

## Running the Beacon Ecosystem
- Building Beacon is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.
- Running Beacon is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.
- Analyzing Beacon stats is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.

## The Applications

The Beacon project consists of the following core applications:
- md_creator
- md_server
- md_client
- ex_engine
- ex_match

## Building the Project
To build the project, use the `build.py` script located at the top level. This script builds all apps sequentially.

### Usage
```bash
build.py [defaults to debug]
build.py debug|release|all|clean|pipeline
```
- **Actions**:
  - `debug` builds include :
    - ASan
    - clang-tidy
    - cppcheck
    - runs all googletest unit and integration tests
  - `release`builds include :
    - -O3 -march=native -mtune=native -DNDEBUG -std=c++20 -flto -funroll-loops -fno-omit-frame-pointer
    - runs all googletest unit and integration tests
  - `all`    : Builds both debug and release.
  - `clean`  : Cleans all build artifacts.
  - `pipeline`: Creates a new release using Beacon's CI/CD pipeline**

### Binary Locations
- Debug binaries: `bin/<app_name>/debug`
- Release binaries: `bin/<app_name>/release`

## Contributing
Please ensure all changes are well-documented and tested. Use the scripts in the `scripts/` directory to streamline your workflow.

## License
Standard MIT License
