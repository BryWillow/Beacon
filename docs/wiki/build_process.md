# Building Beacon

## Prerequisites
[Python](https://www.python.org/downloads/) v3.7 (circa 2018) or higher is required. Beacon's custom *CI/CD pipeline* was built purely in Python.

## Getting Started
Only two steps are required to build Beacon:
```.
git clone https://github.com/BryWillow/Beacon
./build.py all
```
This creates a fresh copy of the Beacon repo, and builds *every app* in the ecosystem.

Your binaries are then placed at:
**/bin/<app_name>/<build_configuraiton>**
```
├── bin
│   └── file_generator
│       ├── debug
│       │   └── file_generator
│       └── release
│           └── file_generator
```

## Build Script
Beacon's build script, build.py

```.
./build.py          # no parameters, defaults to debug
./build.py debug    # debug only: ASan, tidy, cppcheck
./build.py release  # release only: O3 and LTO
./build.py all      # builds both debug and release
./build.py clean    # clean up all build artifacts
./build.py pipeline # create release via CI/CD pipeline
```
## Third-Party Dependencies
Note that Beacon does have a couple of third-party dependencies, both located in the /vendor directory. Howwever, ***no actions*** are required on your part. GoogleTest is packaged with Beacon for convenience, and nlohmann/json is a header file-only dependency.

- **[Google Test](https://google.github.io/googletest)**
- **[nlohmann/json](https://github.com/nlohmann/json)**