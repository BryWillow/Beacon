#!/bin/bash
# End-to-End Demo: Market Data → Algorithm → Matching Engine
# Shows the complete flow with algo_template

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Beacon End-to-End Algorithm Demo${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Configuration
MCAST_ADDR="239.255.0.1"
MCAST_PORT="12345"
EXCHANGE_HOST="127.0.0.1"
EXCHANGE_PORT="9000"
TEST_DURATION_SEC=60

MD_DATA_FILE="../exchange_market_data_generator/output.mdp"
MD_PLAYBACK_CONFIG="../exchange_market_data_playback/config_udp_slow.json"

# Directories
MATCHING_ENGINE_DIR="../exchange_matching_engine"
MD_PLAYBACK_DIR="../exchange_market_data_playback"
CLIENT_ALGO_DIR="."

# Check prerequisites
echo -e "${YELLOW}[1/6] Checking prerequisites...${NC}"

if [ ! -f "${MATCHING_ENGINE_DIR}/build/exchange_matching_engine" ]; then
    echo -e "${RED}ERROR: Matching engine not built${NC}"
    echo "Run: cd ${MATCHING_ENGINE_DIR} && ./build.sh"
    exit 1
fi

if [ ! -f "${MD_PLAYBACK_DIR}/build/exchange_market_data_playback" ]; then
    echo -e "${RED}ERROR: Market data playback not built${NC}"
    echo "Run: cd ${MD_PLAYBACK_DIR} && ./build.sh"
    exit 1
fi

if [ ! -f "${CLIENT_ALGO_DIR}/build/algo_template" ]; then
    echo -e "${RED}ERROR: algo_template not built${NC}"
    echo "Run: cd ${CLIENT_ALGO_DIR} && ./build.sh"
    exit 1
fi

if [ ! -f "${MD_DATA_FILE}" ]; then
    echo -e "${RED}ERROR: Market data file not found: ${MD_DATA_FILE}${NC}"
    echo "Run: cd ../../exchange_market_data_generator && ./run.sh"
    exit 1
fi

echo -e "${GREEN}✓ All components built and ready${NC}"
echo ""

# Start matching engine
echo -e "${YELLOW}[2/6] Starting matching engine on port ${EXCHANGE_PORT}...${NC}"
cd "${MATCHING_ENGINE_DIR}"
./build/exchange_matching_engine "${EXCHANGE_PORT}" > /tmp/beacon_matching_engine.log 2>&1 &
MATCHING_ENGINE_PID=$!
cd - > /dev/null
sleep 1

if ! kill -0 ${MATCHING_ENGINE_PID} 2>/dev/null; then
    echo -e "${RED}ERROR: Matching engine failed to start${NC}"
    cat /tmp/beacon_matching_engine.log
    exit 1
fi

echo -e "${GREEN}✓ Matching engine running (PID: ${MATCHING_ENGINE_PID})${NC}"
echo ""

# Start market data playback
echo -e "${YELLOW}[3/6] Starting market data playback...${NC}"
cd "${MD_PLAYBACK_DIR}"
./build/exchange_market_data_playback \
    --config "${MD_PLAYBACK_CONFIG}" \
    "${MD_DATA_FILE}" > /tmp/beacon_market_data.log 2>&1 &
MD_PLAYBACK_PID=$!
cd - > /dev/null
sleep 1

if ! kill -0 ${MD_PLAYBACK_PID} 2>/dev/null; then
    echo -e "${RED}ERROR: Market data playback failed to start${NC}"
    cat /tmp/beacon_market_data.log
    kill ${MATCHING_ENGINE_PID} 2>/dev/null || true
    exit 1
fi

echo -e "${GREEN}✓ Market data playback running (PID: ${MD_PLAYBACK_PID})${NC}"
echo ""

# Give it a moment to start publishing
sleep 2

# Start algorithm
echo -e "${YELLOW}[4/6] Starting algo_template...${NC}"
echo -e "${BLUE}Configuration:${NC}"
echo "  - Multicast: ${MCAST_ADDR}:${MCAST_PORT}"
echo "  - Exchange: ${EXCHANGE_HOST}:${EXCHANGE_PORT}"
echo "  - Duration: ${TEST_DURATION_SEC} seconds"
echo ""

cd "${CLIENT_ALGO_DIR}"
./build/algo_template \
    "${MCAST_ADDR}" \
    "${MCAST_PORT}" \
    "${EXCHANGE_HOST}" \
    "${EXCHANGE_PORT}" \
    "${TEST_DURATION_SEC}" 2>&1 | tee /tmp/beacon_algo.log &
ALGO_PID=$!
cd - > /dev/null

echo -e "${GREEN}✓ Algorithm started (PID: ${ALGO_PID})${NC}"
echo ""

# Monitor for a bit
echo -e "${YELLOW}[5/6] Running test for ${TEST_DURATION_SEC} seconds...${NC}"
echo -e "${BLUE}(Press Ctrl+C to stop early)${NC}"
echo ""

# Wait for algorithm to complete or user interrupt
trap "echo ''; echo -e '${YELLOW}Interrupted by user${NC}'; kill ${ALGO_PID} ${MD_PLAYBACK_PID} ${MATCHING_ENGINE_PID} 2>/dev/null || true; exit 1" INT

wait ${ALGO_PID} 2>/dev/null || true

# Cleanup
echo ""
echo -e "${YELLOW}[6/6] Cleaning up...${NC}"

kill ${MD_PLAYBACK_PID} 2>/dev/null || true
kill ${MATCHING_ENGINE_PID} 2>/dev/null || true

# Wait a moment for clean shutdown
sleep 1

echo -e "${GREEN}✓ All processes stopped${NC}"
echo ""

# Show results
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Test Results${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if [ -f /tmp/beacon_algo.log ]; then
    echo -e "${YELLOW}Algorithm output (last 30 lines):${NC}"
    tail -30 /tmp/beacon_algo.log
    echo ""
fi

echo -e "${GREEN}✓ End-to-end test complete!${NC}"
echo ""
echo -e "${BLUE}Log files:${NC}"
echo "  - Algorithm:       /tmp/beacon_algo.log"
echo "  - Matching Engine: /tmp/beacon_matching_engine.log"
echo "  - Market Data:     /tmp/beacon_market_data.log"
echo ""
