#!/bin/bash
# =============================================================================
# Project:      Beacon
# File:         run_tests.sh
# Purpose:      Build and run all GoogleTest suites
# Author:       Bryan Camp
# =============================================================================

set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}==========================================${NC}"
echo -e "${BLUE}  Beacon Test Suite${NC}"
echo -e "${BLUE}==========================================${NC}"
echo ""

# Build tests
echo -e "${BLUE}Building tests...${NC}"
cd build
cmake ..
make -j8

echo ""
echo -e "${BLUE}==========================================${NC}"
echo -e "${BLUE}  Running Tests${NC}"
echo -e "${BLUE}==========================================${NC}"
echo ""

# Run all tests
ctest --output-on-failure --verbose

echo ""
echo -e "${GREEN}==========================================${NC}"
echo -e "${GREEN}  Test Suite Complete!${NC}"
echo -e "${GREEN}==========================================${NC}"
