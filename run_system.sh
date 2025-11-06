#!/bin/bash

# =============================================================================
# Project:      Beacon
# Script:       run_system.sh
# Purpose:      Launch the complete Beacon trading system
# Author:       Bryan Camp
# Usage:        ./run_system.sh [generator|playback] [config_file]
# =============================================================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Parse arguments
MD_SOURCE=${1:-generator}  # generator or playback
ALGO_CONFIG=${2:-src/apps/client_algorithm/sample_config.json}

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}  Beacon Trading System - System Launcher  ${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""
echo -e "${CYAN}Configuration:${NC}"
echo -e "  Market Data Source: ${YELLOW}${MD_SOURCE}${NC}"
echo -e "  Algorithm Config:   ${YELLOW}${ALGO_CONFIG}${NC}"
echo ""

# Check if binaries exist
check_binary() {
    local bin_path=$1
    local app_name=$2
    
    if [ ! -f "$bin_path" ]; then
        echo -e "${RED}Error: ${app_name} binary not found at ${bin_path}${NC}"
        echo -e "${YELLOW}Please run ./build_all.sh first${NC}"
        exit 1
    fi
}

check_binary "src/apps/exchange_matching_engine/build/matching_engine" "Matching Engine"
check_binary "src/apps/client_algorithm/build/client_algo" "Client Algorithm"

if [ "$MD_SOURCE" = "generator" ]; then
    check_binary "src/apps/exchange_market_data_generator/build/md_generator" "Market Data Generator"
elif [ "$MD_SOURCE" = "playback" ]; then
    check_binary "src/apps/exchange_market_data_playback/build/md_playback" "Market Data Playback"
else
    echo -e "${RED}Error: Invalid market data source: ${MD_SOURCE}${NC}"
    echo -e "${YELLOW}Use 'generator' or 'playback'${NC}"
    exit 1
fi

# Check if algorithm config exists
if [ ! -f "$ALGO_CONFIG" ]; then
    echo -e "${RED}Error: Algorithm config not found: ${ALGO_CONFIG}${NC}"
    exit 1
fi

echo -e "${GREEN}All binaries found!${NC}"
echo ""

# Create log directory
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo -e "${CYAN}Logs will be written to: ${LOG_DIR}${NC}"
echo ""

# Cleanup function
cleanup() {
    echo ""
    echo -e "${YELLOW}Shutting down Beacon Trading System...${NC}"
    
    if [ ! -z "$ALGO_PID" ]; then
        echo -e "${YELLOW}  Stopping Client Algorithm (PID: $ALGO_PID)...${NC}"
        kill $ALGO_PID 2>/dev/null || true
    fi
    
    if [ ! -z "$MD_PID" ]; then
        echo -e "${YELLOW}  Stopping Market Data Source (PID: $MD_PID)...${NC}"
        kill $MD_PID 2>/dev/null || true
    fi
    
    if [ ! -z "$ENGINE_PID" ]; then
        echo -e "${YELLOW}  Stopping Matching Engine (PID: $ENGINE_PID)...${NC}"
        kill $ENGINE_PID 2>/dev/null || true
    fi
    
    echo -e "${GREEN}Shutdown complete${NC}"
    exit 0
}

trap cleanup SIGINT SIGTERM

# Start Matching Engine
echo -e "${GREEN}[1/3] Starting Matching Engine...${NC}"
cd "$SCRIPT_DIR/src/apps/exchange_matching_engine"
./build/matching_engine > "$LOG_DIR/matching_engine_${TIMESTAMP}.log" 2>&1 &
ENGINE_PID=$!
echo -e "${CYAN}  PID: ${ENGINE_PID}${NC}"
echo -e "${CYAN}  Log: ${LOG_DIR}/matching_engine_${TIMESTAMP}.log${NC}"
sleep 2  # Give it time to start listening

# Start Market Data Source
echo ""
if [ "$MD_SOURCE" = "generator" ]; then
    echo -e "${GREEN}[2/3] Starting Market Data Generator...${NC}"
    cd "$SCRIPT_DIR/src/apps/exchange_market_data_generator"
    if [ -f "run.sh" ]; then
        ./run.sh > "$LOG_DIR/md_generator_${TIMESTAMP}.log" 2>&1 &
    else
        ./build/md_generator > "$LOG_DIR/md_generator_${TIMESTAMP}.log" 2>&1 &
    fi
    MD_PID=$!
    echo -e "${CYAN}  PID: ${MD_PID}${NC}"
    echo -e "${CYAN}  Log: ${LOG_DIR}/md_generator_${TIMESTAMP}.log${NC}"
else
    echo -e "${GREEN}[2/3] Starting Market Data Playback...${NC}"
    cd "$SCRIPT_DIR/src/apps/exchange_market_data_playback"
    # Note: You may need to specify a data file here
    echo -e "${YELLOW}  Note: Playback mode requires a data file${NC}"
    echo -e "${YELLOW}  Modify this script to specify your data file${NC}"
    # Uncomment and modify:
    # ./build/md_playback <your_data_file> 239.255.0.1 12345 > "$LOG_DIR/md_playback_${TIMESTAMP}.log" 2>&1 &
    # MD_PID=$!
fi
sleep 2  # Give market data time to start

# Start Client Algorithm
echo ""
echo -e "${GREEN}[3/3] Starting Client Algorithm...${NC}"
cd "$SCRIPT_DIR/src/apps/client_algorithm"
./build/client_algo --config "$SCRIPT_DIR/$ALGO_CONFIG" > "$LOG_DIR/client_algo_${TIMESTAMP}.log" 2>&1 &
ALGO_PID=$!
echo -e "${CYAN}  PID: ${ALGO_PID}${NC}"
echo -e "${CYAN}  Log: ${LOG_DIR}/client_algo_${TIMESTAMP}.log${NC}"

echo ""
echo -e "${BLUE}============================================${NC}"
echo -e "${GREEN}Beacon Trading System is now running!${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""
echo -e "${YELLOW}Component PIDs:${NC}"
echo -e "  Matching Engine:      ${CYAN}${ENGINE_PID}${NC}"
echo -e "  Market Data Source:   ${CYAN}${MD_PID}${NC}"
echo -e "  Client Algorithm:     ${CYAN}${ALGO_PID}${NC}"
echo ""
echo -e "${YELLOW}To monitor logs:${NC}"
echo -e "  tail -f ${LOG_DIR}/matching_engine_${TIMESTAMP}.log"
echo -e "  tail -f ${LOG_DIR}/md_generator_${TIMESTAMP}.log"
echo -e "  tail -f ${LOG_DIR}/client_algo_${TIMESTAMP}.log"
echo ""
echo -e "${YELLOW}Press Ctrl+C to shutdown all components${NC}"
echo ""

# Wait for processes
wait
