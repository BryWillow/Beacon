#!/bin/bash

# =============================================================================
# Project:      Beacon
# Script:       build_all.sh
# Purpose:      Build all Beacon applications in one command
# Author:       Bryan Camp
# =============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}  Beacon Trading System - Build All ${NC}"
echo -e "${BLUE}=====================================${NC}"
echo ""

# Function to build an app
build_app() {
    local app_name=$1
    local app_path=$2
    
    echo -e "${YELLOW}Building ${app_name}...${NC}"
    
    cd "$SCRIPT_DIR/$app_path"
    
    if [ -f "build.sh" ]; then
        chmod +x build.sh
        ./build.sh
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ ${app_name} built successfully${NC}"
        else
            echo -e "${RED}✗ ${app_name} build failed${NC}"
            return 1
        fi
    else
        echo -e "${YELLOW}  No build.sh found, using CMake directly...${NC}"
        mkdir -p build
        cd build
        cmake ..
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ ${app_name} built successfully${NC}"
        else
            echo -e "${RED}✗ ${app_name} build failed${NC}"
            return 1
        fi
    fi
    
    echo ""
    cd "$SCRIPT_DIR"
}

# Build all applications
echo -e "${BLUE}Building Applications...${NC}"
echo ""

build_app "Market Data Generator" "src/apps/exchange_market_data_generator"
build_app "Market Data Playback" "src/apps/exchange_market_data_playback"
build_app "Matching Engine" "src/apps/exchange_matching_engine"
build_app "Client Algorithm" "src/apps/client_algorithm"

echo -e "${BLUE}=====================================${NC}"
echo -e "${GREEN}All builds completed successfully!${NC}"
echo -e "${BLUE}=====================================${NC}"
echo ""
echo -e "${YELLOW}Binary locations:${NC}"
echo "  Market Data Generator: src/apps/exchange_market_data_generator/build/md_generator"
echo "  Market Data Playback:  src/apps/exchange_market_data_playback/build/md_playback"
echo "  Matching Engine:       src/apps/exchange_matching_engine/build/matching_engine"
echo "  Client Algorithm:      src/apps/client_algorithm/build/client_algo"
echo ""
echo -e "${YELLOW}To run the full system:${NC}"
echo "  Terminal 1: cd src/apps/exchange_matching_engine && ./build/matching_engine"
echo "  Terminal 2: cd src/apps/exchange_market_data_generator && ./run.sh"
echo "  Terminal 3: cd src/apps/client_algorithm && ./build/client_algo --config sample_config.json"
echo ""
