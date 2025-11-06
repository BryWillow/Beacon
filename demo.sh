#!/bin/bash
# Beacon System Demo - Complete End-to-End Test
# Usage: ./demo.sh [duration_seconds]
# Default duration: 10 seconds

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DURATION=${1:-10}  # Default 10 seconds
MCAST_ADDR="239.255.0.1"
MCAST_PORT="12345"
EXCHANGE_HOST="127.0.0.1"
EXCHANGE_PORT="9000"

# Paths
MATCHING_ENGINE="src/apps/exchange_matching_engine/build/exchange_matching_engine"
MD_PLAYBACK="src/apps/exchange_market_data_playback/build/exchange_market_data_playback"
MD_CONFIG="src/apps/exchange_market_data_playback/config_udp_slow.json"
MD_FILE="src/apps/exchange_market_data_generator/output.mdp"
ALGO_TEMPLATE="src/apps/client_algorithm/build/algo_template"

echo ""
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${CYAN}                   BEACON SYSTEM DEMO                            ${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
echo -e "${BLUE}Configuration:${NC}"
echo "  Duration:     ${DURATION} seconds"
echo "  Multicast:    ${MCAST_ADDR}:${MCAST_PORT}"
echo "  Exchange:     ${EXCHANGE_HOST}:${EXCHANGE_PORT}"
echo ""

# Clean up any existing processes
echo -e "${YELLOW}Cleaning up existing processes...${NC}"
./kill_all.sh > /dev/null 2>&1 || true
sleep 1

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"
MISSING=0

if [ ! -f "$MATCHING_ENGINE" ]; then
    echo -e "${RED}  ✗ Matching engine not built${NC}"
    echo "    Run: cd src/apps/exchange_matching_engine && ./build.sh"
    MISSING=1
fi

if [ ! -f "$MD_PLAYBACK" ]; then
    echo -e "${RED}  ✗ Market data playback not built${NC}"
    echo "    Run: cd src/apps/exchange_market_data_playback && ./build.sh"
    MISSING=1
fi

if [ ! -f "$ALGO_TEMPLATE" ]; then
    echo -e "${RED}  ✗ Algorithm template not built${NC}"
    echo "    Run: cd src/apps/client_algorithm && ./build.sh"
    MISSING=1
fi

if [ ! -f "$MD_FILE" ]; then
    echo -e "${RED}  ✗ Market data file not found${NC}"
    echo "    Run: cd src/apps/exchange_market_data_generator && ./run.sh"
    MISSING=1
fi

if [ $MISSING -eq 1 ]; then
    echo ""
    echo -e "${RED}Missing components. Please build them first.${NC}"
    exit 1
fi

echo -e "${GREEN}  ✓ All components ready${NC}"
echo ""

# Trap to cleanup on exit
cleanup() {
    echo ""
    echo -e "${YELLOW}Shutting down...${NC}"
    # Kill all child processes
    if [ ! -z "$ME_PID" ]; then kill -9 $ME_PID 2>/dev/null || true; fi
    if [ ! -z "$MD_PID" ]; then kill -9 $MD_PID 2>/dev/null || true; fi
    if [ ! -z "$ALGO_PID" ]; then kill -9 $ALGO_PID 2>/dev/null || true; fi
    # Also use kill_all to be sure
    ./kill_all.sh > /dev/null 2>&1 || true
    echo -e "${GREEN}✓ All processes stopped${NC}"
}
trap cleanup EXIT INT TERM

# Start components in order
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${CYAN}                     STARTING COMPONENTS                         ${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# [1] Start Matching Engine
$MATCHING_ENGINE $EXCHANGE_PORT &
ME_PID=$!
sleep 1

if ! kill -0 $ME_PID 2>/dev/null; then
    echo -e "${RED}ERROR: Matching engine failed to start${NC}"
    exit 1
fi

# [2] Already started (matching engine prints [2])

# [5] Start Market Data Playback
$MD_PLAYBACK --config "$MD_CONFIG" "$MD_FILE" > /tmp/beacon_md_playback.log 2>&1 &
MD_PID=$!
sleep 2

if ! kill -0 $MD_PID 2>/dev/null; then
    echo -e "${RED}ERROR: Market data playback failed to start${NC}"
    cat /tmp/beacon_md_playback.log
    exit 1
fi

# [6] Already started (playback prints [6])
echo ""

# [3][4] Start Algorithm
echo -e "${YELLOW}Starting algorithm (will run for ${DURATION} seconds)...${NC}"
$ALGO_TEMPLATE $MCAST_ADDR $MCAST_PORT $EXCHANGE_HOST $EXCHANGE_PORT $DURATION &
ALGO_PID=$!

# Wait for algorithm to complete or timeout
WAITED=0
while kill -0 $ALGO_PID 2>/dev/null; do
    sleep 1
    WAITED=$((WAITED + 1))
    if [ $WAITED -gt $((DURATION + 5)) ]; then
        echo -e "${RED}Algorithm timeout - forcing shutdown${NC}"
        kill -9 $ALGO_PID 2>/dev/null || true
        break
    fi
done

# Show market data playback summary
echo ""
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${CYAN}                  MARKET DATA PLAYBACK SUMMARY                   ${NC}"
echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
tail -20 /tmp/beacon_md_playback.log | grep -E "Progress|COMPLETE|Duration|Sent|Rate" || echo "See /tmp/beacon_md_playback.log for details"
echo ""

echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}                      DEMO COMPLETE!                             ${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
echo "Logs available at:"
echo "  - Market Data: /tmp/beacon_md_playback.log"
echo ""
