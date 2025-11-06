#!/bin/bash
# Brute force cleanup script - kills all Beacon experimental processes
# Usage: ./kill_all.sh

set +e  # Don't exit on errors

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Beacon Process Cleanup${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Process patterns to search for
PATTERNS=(
    "exchange_matching_engine"
    "exchange_market_data_playback"
    "exchange_market_data_generator"
    "client_algorithm"
    "client_algo"
    "algo_twap"
    "algo_template"
    "algo_vwap"
    "udp_listener.py"
    "test_pillar"
    "test_cme"
)

TOTAL_FOUND=0
TOTAL_KILLED=0
TOTAL_FAILED=0

echo -e "${YELLOW}Scanning for running processes...${NC}"
echo ""

# Find all matching processes
declare -a PIDS_TO_KILL
declare -a PROCESS_NAMES

for pattern in "${PATTERNS[@]}"; do
    # Find PIDs matching this pattern (exclude grep and this script)
    PIDS=$(pgrep -f "$pattern" | grep -v "$$")
    
    if [ -n "$PIDS" ]; then
        for pid in $PIDS; do
            # Get process info
            PROCESS_INFO=$(ps -p "$pid" -o comm= 2>/dev/null)
            
            if [ -n "$PROCESS_INFO" ]; then
                PIDS_TO_KILL+=("$pid")
                PROCESS_NAMES+=("$PROCESS_INFO")
                ((TOTAL_FOUND++))
                echo -e "${BLUE}[FOUND]${NC} PID ${pid}: ${PROCESS_INFO}"
            fi
        done
    fi
done

echo ""

if [ $TOTAL_FOUND -eq 0 ]; then
    echo -e "${GREEN}✓ No Beacon processes found running${NC}"
    echo ""
    exit 0
fi

echo -e "${YELLOW}Found ${TOTAL_FOUND} process(es) to kill${NC}"
echo ""

# Kill all found processes
echo -e "${YELLOW}Attempting to kill processes...${NC}"
echo ""

for i in "${!PIDS_TO_KILL[@]}"; do
    pid="${PIDS_TO_KILL[$i]}"
    name="${PROCESS_NAMES[$i]}"
    
    # Try graceful kill first (SIGTERM)
    if kill "$pid" 2>/dev/null; then
        sleep 0.1
        
        # Check if process is still running
        if kill -0 "$pid" 2>/dev/null; then
            # Process still running, try SIGKILL
            if kill -9 "$pid" 2>/dev/null; then
                echo -e "${GREEN}[KILLED]${NC} PID ${pid}: ${name} (forced)"
                ((TOTAL_KILLED++))
            else
                echo -e "${RED}[FAILED]${NC} PID ${pid}: ${name} (permission denied?)"
                ((TOTAL_FAILED++))
            fi
        else
            echo -e "${GREEN}[KILLED]${NC} PID ${pid}: ${name}"
            ((TOTAL_KILLED++))
        fi
    else
        # Process already dead or permission denied
        if kill -0 "$pid" 2>/dev/null; then
            echo -e "${RED}[FAILED]${NC} PID ${pid}: ${name} (permission denied)"
            ((TOTAL_FAILED++))
        else
            echo -e "${YELLOW}[GONE]${NC} PID ${pid}: ${name} (already terminated)"
            ((TOTAL_KILLED++))
        fi
    fi
done

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Cleanup Summary${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "  Found:    ${TOTAL_FOUND}"
echo -e "  Killed:   ${GREEN}${TOTAL_KILLED}${NC}"
echo -e "  Failed:   ${RED}${TOTAL_FAILED}${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check for lingering processes
echo -e "${YELLOW}Verifying cleanup...${NC}"
REMAINING=0
for pattern in "${PATTERNS[@]}"; do
    PIDS=$(pgrep -f "$pattern" | grep -v "$$")
    if [ -n "$PIDS" ]; then
        for pid in $PIDS; do
            PROCESS_INFO=$(ps -p "$pid" -o comm= 2>/dev/null)
            if [ -n "$PROCESS_INFO" ]; then
                echo -e "${RED}[STILL RUNNING]${NC} PID ${pid}: ${PROCESS_INFO}"
                ((REMAINING++))
            fi
        done
    fi
done

echo ""

if [ $REMAINING -eq 0 ]; then
    echo -e "${GREEN}✓ All processes successfully terminated${NC}"
else
    echo -e "${RED}✗ ${REMAINING} process(es) still running (may require sudo)${NC}"
fi

# Check for lingering sockets
echo ""
echo -e "${YELLOW}Checking for lingering sockets...${NC}"
SOCKETS=$(lsof -i :9000 -i :12345 2>/dev/null | grep -v COMMAND)
if [ -n "$SOCKETS" ]; then
    echo -e "${YELLOW}Found bound sockets:${NC}"
    echo "$SOCKETS"
    echo -e "${YELLOW}(Will be cleaned up when processes exit)${NC}"
else
    echo -e "${GREEN}✓ No sockets bound to ports 9000 or 12345${NC}"
fi

# Check for temp log files
echo ""
echo -e "${YELLOW}Checking for temporary log files...${NC}"
LOG_FILES=$(ls -lh /tmp/beacon_*.log 2>/dev/null)
if [ -n "$LOG_FILES" ]; then
    echo -e "${BLUE}Found log files:${NC}"
    ls -lh /tmp/beacon_*.log 2>/dev/null
    echo ""
    read -p "Delete log files? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -f /tmp/beacon_*.log
        echo -e "${GREEN}✓ Log files deleted${NC}"
    else
        echo -e "${YELLOW}Log files kept${NC}"
    fi
else
    echo -e "${GREEN}✓ No temporary log files found${NC}"
fi

echo ""
if [ $REMAINING -eq 0 ]; then
    exit 0
else
    exit 1
fi
