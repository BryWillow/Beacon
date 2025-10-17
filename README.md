# The Beacon Project

## Why Beacon?
The word "Beacon" represents a guiding light—a source of hope for solving tough challenges. This project serves as a platform to explore, innovate, and measure progress in the HFT landscape.

## Project Overview
The Beacon project is an ecosystem of modular C++ applications, each designed with a single purpose but working seamlessly together. From ingesting market data at bursty, configurable rates to strategy testing, risk management, order execution, and reporting, the system is built to:
- Efficiently measure latency at every stage.
- Offer a pluggable, highly testable architecture.
- Provide optimized implementations by default.

## Directory Structure
- **src/**: The source code for each application.
- **include/**: The codebase is heavily header-based.
- **bin/**: The directory where your compiled binaries are placed.
- **build/**: Contains intermediate build files for each app.
- **scripts/**: Contains Pipeilne scripts.
  - **build/**: Build scripts for each separate application.
  - **release/**: Scripts for release-related tasks, such as generating release notes and managing version control.
- **utils/**: Utility scripts and tools.

## Building the Project
To build the project, use the `build.sh` script located at the top level. This script builds all apps sequentially.

### Usage
```bash
./build.sh [action]
```
- **Actions**:
  - `debug`: Builds all apps in debug mode.
  - `release`: Builds all apps in release mode.
  - `all`: Builds all apps in both debug and release modes.
  - `clean`: Cleans all build artifacts.

### Binary Locations
- Debug binaries: `bin/<app_name>/debug`
- Release binaries: `bin/<app_name>/release`

## Contributing
Please ensure all changes are well-documented and tested. Use the scripts in the `scripts/` directory to streamline your workflow.

## License
[Insert license information here]
