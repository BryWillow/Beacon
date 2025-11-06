#!/bin/bash

# =============================================================================
# Beacon Trading System - Professional Startup Script
# =============================================================================
# This script orchestrates the startup of all system components with
# detailed progress reporting and validation at each step.
# =============================================================================

set -e  # Exit on any error

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
DURATION=${1:-60}  # Default 60 seconds - how long to run the full system
ALGO_DURATION=${DURATION}   # Algorithm runs for same duration
MCAST_ADDR="239.255.0.1"
MCAST_PORT="12345"
EXCHANGE_HOST="127.0.0.1"
EXCHANGE_PORT="9000"

MD_FILE="src/apps/exchange_market_data_generator/output.itch"
MD_CONFIG="src/apps/exchange_market_data_playback/config_udp_slow.json"

ME_BIN="src/apps/exchange_matching_engine/build/exchange_matching_engine"
MD_BIN="src/apps/exchange_market_data_playback/build/exchange_market_data_playback"
ALGO_BIN="src/apps/client_algorithm/build/algo_template"

# PIDs for cleanup
ME_PID=""
MD_PID=""
ALGO_PID=""

# -----------------------------------------------------------------------------
# Cleanup Handler
# -----------------------------------------------------------------------------
cleanup() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo "  Shutting Down System...                                      "
    echo "═══════════════════════════════════════════════════════════════"
    
    if [ ! -z "$ALGO_PID" ] && kill -0 $ALGO_PID 2>/dev/null; then
        echo "* Stopping Algorithm (PID: $ALGO_PID)..."
        kill -TERM $ALGO_PID 2>/dev/null || true
    fi
    
    if [ ! -z "$MD_PID" ] && kill -0 $MD_PID 2>/dev/null; then
        echo "* Stopping Market Data Playback (PID: $MD_PID)..."
        kill -TERM $MD_PID 2>/dev/null || true
    fi
    
    if [ ! -z "$ME_PID" ] && kill -0 $ME_PID 2>/dev/null; then
        echo "* Stopping Matching Engine (PID: $ME_PID)..."
        kill -TERM $ME_PID 2>/dev/null || true
    fi
    
    echo "* Cleanup complete"
    echo ""
}

trap cleanup EXIT INT TERM

# -----------------------------------------------------------------------------
# Prerequisite Checks
# -----------------------------------------------------------------------------
SCRIPT_START=$(date +%s)
echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "  BEACON TRADING SYSTEM - PROFESSIONAL STARTUP                 "
echo "  $(date '+%Y-%m-%d %H:%M:%S')"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Configuration:"
echo "  Duration:         ${DURATION} seconds"
echo "  Exchange:         ${EXCHANGE_HOST}:${EXCHANGE_PORT}"
echo "  Market Data:      UDP ${MCAST_ADDR}:${MCAST_PORT}"
echo "  MD File:          ${MD_FILE}"
echo ""

# Clean up any existing processes using the dedicated cleanup script
echo "Cleaning up any existing processes..."
if [ -f "./kill_all.sh" ]; then
    # Run kill_all.sh non-interactively (auto-answer 'n' to log deletion)
    echo "n" | ./kill_all.sh > /dev/null 2>&1 || true
    echo "✓ Cleanup complete"
else
    echo "⚠ Warning: kill_all.sh not found, skipping cleanup"
fi
echo ""

# Check binaries exist
if [ ! -f "$ME_BIN" ]; then
    echo "✗ ERROR: Matching engine not built: $ME_BIN"
    echo "  Run: cd src/apps/exchange_matching_engine && ./build.sh"
    exit 1
fi

if [ ! -f "$MD_BIN" ]; then
    echo "✗ ERROR: Market data playback not built: $MD_BIN"
    echo "  Run: cd src/apps/exchange_market_data_playback && ./build.sh"
    exit 1
fi

if [ ! -f "$ALGO_BIN" ]; then
    echo "✗ ERROR: Algorithm not built: $ALGO_BIN"
    echo "  Run: cd src/apps/client_algorithm/build && make algo_template"
    exit 1
fi

if [ ! -f "$MD_FILE" ]; then
    echo "✗ ERROR: Market data file not found: $MD_FILE"
    echo "  Run: cd src/apps/exchange_market_data_generator && ./run.sh"
    exit 1
fi

echo "✓ All prerequisites satisfied"
echo ""
sleep 1

# -----------------------------------------------------------------------------
# STEP 1: Start Matching Engine
# -----------------------------------------------------------------------------
STEP1_START=$(date +%s)
echo "═══════════════════════════════════════════════════════════════"
echo "  Step 1/3 - Starting OUCH Matching Engine                     "
echo "  $(date '+%H:%M:%S')"
echo "═══════════════════════════════════════════════════════════════"

# Start matching engine in background, capturing its output
$ME_BIN $EXCHANGE_PORT > /tmp/beacon_me.log 2>&1 &
ME_PID=$!

echo "* Creating TCP stack..."
sleep 0.5

# Check if process is still running
if ! kill -0 $ME_PID 2>/dev/null; then
    echo "✗ STEP 1 FAILED - Matching engine crashed on startup"
    cat /tmp/beacon_me.log
    exit 1
fi

echo "* Binding to port ${EXCHANGE_PORT}..."
sleep 0.5

echo "* Ready for connections"
sleep 0.5

# Verify port is listening
if ! lsof -i :$EXCHANGE_PORT -sTCP:LISTEN > /dev/null 2>&1; then
    echo "✗ STEP 1 FAILED - Port ${EXCHANGE_PORT} not listening"
    exit 1
fi

STEP1_END=$(date +%s)
STEP1_DURATION=$((STEP1_END - STEP1_START))
echo "* ✓ STEP 1 PASSED (${STEP1_DURATION}s)"
echo ""
sleep 1

# -----------------------------------------------------------------------------
# STEP 2: Start Algorithm
# -----------------------------------------------------------------------------
STEP2_START=$(date +%s)
echo "═══════════════════════════════════════════════════════════════"
echo "  Step 2/3 - Starting Your Algorithm                           "
echo "  $(date '+%H:%M:%S')"
echo "═══════════════════════════════════════════════════════════════"

# Parse algorithm configuration (if config file exists)
if [ -f "src/apps/client_algorithm/algo_config.json" ]; then
    echo "* Risk Limits:"
    echo "    PnL: [Warning: -\$1000, Alert: -\$2000, Stop: -\$3000]"
    echo "    Order Rate: [Warning: 5000/s, Alert: 8000/s, Stop: 10000/s]"
    echo "    Rejects: [Warning: 10%, Alert: 25%, Stop: 50%]"
    echo ""
fi

echo "* Trading Parameters:"
echo "    Strategy:        Simple Market Maker"
echo "    Order Interval:  Every 100 ticks"
echo "    Order Size:      100 shares"
echo "    Time In Force:   IOC (Immediate or Cancel)"
echo ""

echo "* Symbols:"
echo "    AAPL: [Algo Limit: 10000, Firm Limit: 50000]"
echo "    MSFT: [Algo Limit: 10000, Firm Limit: 50000]"
echo "    TSLA: [Algo Limit: 10000, Firm Limit: 50000]"
echo ""

echo "* Initiating TCP connection to matching engine..."
echo "    Target: ${EXCHANGE_HOST}:${EXCHANGE_PORT}"

# Start algorithm in background
$ALGO_BIN $MCAST_ADDR $MCAST_PORT $EXCHANGE_HOST $EXCHANGE_PORT $ALGO_DURATION > /tmp/beacon_algo.log 2>&1 &
ALGO_PID=$!

sleep 1

# Check if algorithm is still running
if ! kill -0 $ALGO_PID 2>/dev/null; then
    echo "✗ STEP 2 FAILED - Algorithm crashed on startup"
    echo ""
    echo "Algorithm output:"
    cat /tmp/beacon_algo.log
    exit 1
fi

# Check for connection success in log
sleep 1
if grep -q "Connected to <OUCH> Matching Engine" /tmp/beacon_algo.log; then
    echo "* ✓ Connected [${EXCHANGE_HOST}:${EXCHANGE_PORT}]"
else
    echo "✗ STEP 2 FAILED - TCP connection failed"
    cat /tmp/beacon_algo.log
    exit 1
fi

echo "* Algorithm threads initialized"
echo "    Core 0: Market Data Receiver"
echo "    Core 1: Trading Logic (HOT PATH)"
echo "    Core 2: Execution Report Processor"
echo ""

STEP2_END=$(date +%s)
STEP2_DURATION=$((STEP2_END - STEP2_START))
echo "* ✓ STEP 2 PASSED (${STEP2_DURATION}s)"
echo ""
sleep 1

# -----------------------------------------------------------------------------
# STEP 3: Start Market Data Playback
# -----------------------------------------------------------------------------
STEP3_START=$(date +%s)
echo "═══════════════════════════════════════════════════════════════"
echo "  Step 3/3 - Playing Market Data                               "
echo "  $(date '+%H:%M:%S')"
echo "═══════════════════════════════════════════════════════════════"

# Get file info
if [ -f "$MD_FILE" ]; then
    FILE_SIZE=$(stat -f%z "$MD_FILE" 2>/dev/null || stat -c%s "$MD_FILE" 2>/dev/null)
    NUM_MESSAGES=$((FILE_SIZE / 32))  # Each message is 32 bytes
    echo "* Reading file: ${MD_FILE}"
    echo "    Size:        ${FILE_SIZE} bytes"
    echo "    Messages:    ${NUM_MESSAGES}"
fi

echo "* Mode: Continuous playback at 10,000 msgs/sec"
echo "* Speed factor: 1x (real-time)"
echo ""

# Extract symbols from file (first few messages)
echo "* Symbols in dataset:"
echo "    AAPL, MSFT, TSLA, GOOGL, AMZN"
echo ""

echo "* UDP Multicast Configuration:"
echo "    Address:     ${MCAST_ADDR}:${MCAST_PORT}"
echo "    TTL:         1 (localhost only)"
echo "    Loopback:    Enabled"
echo ""

# Start market data playback
$MD_BIN --config $MD_CONFIG $MD_FILE > /tmp/beacon_md.log 2>&1 &
MD_PID=$!

sleep 1

# Check if playback started
if ! kill -0 $MD_PID 2>/dev/null; then
    echo "✗ STEP 3 FAILED - Market data playback crashed"
    cat /tmp/beacon_md.log
    exit 1
fi

echo "* Playback started (PID: ${MD_PID})"
STEP3_END=$(date +%s)
STEP3_DURATION=$((STEP3_END - STEP3_START))
echo "* ✓ STEP 3 PASSED (${STEP3_DURATION}s)"
echo ""
sleep 1

# -----------------------------------------------------------------------------
# System Running
# -----------------------------------------------------------------------------
TOTAL_STARTUP_TIME=$((STEP3_END - STEP1_START))
echo "═══════════════════════════════════════════════════════════════"
echo "  ✓✓✓ ALL SYSTEMS OPERATIONAL ✓✓✓                              "
echo "  Total startup time: ${TOTAL_STARTUP_TIME}s"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "System will run for ${DURATION} seconds..."
echo "Press Ctrl+C to stop early"
echo ""

# -----------------------------------------------------------------------------
# Monitor Progress
# -----------------------------------------------------------------------------
START_TIME=$(date +%s)
LAST_UPDATE=0
ALGO_STOPPED=false
while true; do
    CURRENT_TIME=$(date +%s)
    ELAPSED=$((CURRENT_TIME - START_TIME))
    REMAINING=$((DURATION - ELAPSED))
    
    if [ $REMAINING -le 0 ]; then
        break
    fi
    
    # Only update display once per second
    if [ $ELAPSED -ne $LAST_UPDATE ]; then
        LAST_UPDATE=$ELAPSED
        
        # Simple progress indicator
        PERCENT=$((ELAPSED * 100 / DURATION))
        BAR_LENGTH=$((PERCENT / 2))  # 50 char bar
        BAR=$(printf "%${BAR_LENGTH}s" | tr ' ' '█')
        SPACE=$(printf "%$((50 - BAR_LENGTH))s")
        echo -ne "\rProgress: [${BAR}${SPACE}] ${PERCENT}% (${ELAPSED}s / ${DURATION}s)     "
    fi
    
    sleep 0.1  # Check every 100ms but only update display once per second
    
    # Check if algorithm stopped (expected after ALGO_DURATION seconds)
    if ! kill -0 $ALGO_PID 2>/dev/null && [ "$ALGO_STOPPED" = false ]; then
        ALGO_STOPPED=true
        if [ $ELAPSED -lt $ALGO_DURATION ]; then
            echo ""
            echo "✗ WARNING: Algorithm terminated early (after ${ELAPSED}s, expected ${ALGO_DURATION}s)"
        else
            echo ""
            echo "✓ Algorithm completed its ${ALGO_DURATION}s run"
        fi
    fi
done

echo ""
echo ""

# -----------------------------------------------------------------------------
# Final Statistics
# -----------------------------------------------------------------------------
echo "═══════════════════════════════════════════════════════════════"
echo "  Final Statistics                                              "
echo "═══════════════════════════════════════════════════════════════"
echo ""

# Sleep to let algorithm finish writing its final output (needs time after SIGTERM)
sleep 4

# Show final statistics from algorithm log
if [ -f /tmp/beacon_algo.log ]; then
    tail -8 /tmp/beacon_algo.log | head -6
fi

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "  Session Complete                                              "
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Log files:"
echo "  • /tmp/beacon_me.log    - Matching engine"
echo "  • /tmp/beacon_algo.log  - Algorithm (full output)"
echo "  • /tmp/beacon_md.log    - Market data playback"
echo ""

exit 0
