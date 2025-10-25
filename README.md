# The Beacon Project

## Why Beacon?
The word "Beacon" represents a guiding light—a source of hope for solving tough challenges. This project serves as a platform to explore, innovate, and measure progress in the HFT landscape.

## Project Overview
The Beacon project is an ecosystem of modular C++ applications, each designed with a single purpose but working seamlessly together. From ingesting market data at bursty, configurable rates to strategy testing, risk management, order execution, and reporting, the system is built to:
- Efficiently measure latency at every stage.
- Offer a pluggable, highly testable architecture.
- Provide optimized implementations by default.

## Beacon Core Applications

| Application  | Purpose
| -----------: | :------
| <a href="docs/wiki/apps/md_creator.md">md_creator</a> | Creates files containing binary messages in exchange format.
| <a href="docs/wiki/apps/md_server.md">md_server</a> | Reads files created by the **md_creator** and broadcasts the messages via UDP.
| <a href="docs/wiki/apps/md_client.md">md_client</a> | Listens for messages sent by the **md_server**. ***<span style="color:red;">Your algo belongs here.</span>***
| <a href="docs/wiki/apps/ex_match.md">ex_match</a> | Maintains market depth. Processes orders. Sends execution reports.

## Building Beacon
To build the project, simply use the `build.py` script located at the top level of the repository. Specifying *all* will build all applications, in both *debug* and *release* mode.

#### Usage:
```bash
./build.py all
```
The build script has several other options, which are outlined <a href="docs/wiki/build/build_process.md">here</a>.
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

## Running Beacon
- Building Beacon is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.
- Running Beacon is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.
- Analyzing Beacon stats is straight-forward. <a href="docs/wiki/build_process.md">Click here</a> to get started.

### Binary Locations
- Debug binaries: `bin/<app_name>/debug`
- Release binaries: `bin/<app_name>/release`

## Contributing
Please ensure all changes are well-documented and tested. Use the scripts in the `scripts/` directory to streamline your workflow.

## License
Standard MIT License
