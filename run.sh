#!/bin/bash
###############################################################################
# @file        run.sh
# @author      Bryan Camp
# @brief       Run Market Data Pipeline: Listener + Replayer + optional generator
# @details     Handles Debug/Release builds, generator parameter validation,
#              listener/replayer startup, and prints a simple ASCII report.
###############################################################################

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_TYPE=${1:-Debug}
BIN_DIR="$PROJECT_ROOT/bin/$BUILD_TYPE"
DATA_DIR="$PROJECT_ROOT/data"

GEN_RUN=false
GEN_ARGS=()
ITCH_FILE=""
MAX_TRIES=5

#==============================#
# Function: prompt_for_generator
# @brief    Prompt user to run generator and collect parameters
#==============================#
prompt_for_generator() {
    echo "Do you want to run the generator? [y/n]"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        GEN_RUN=true
        local tries=0
        while [[ $tries -lt $MAX_TRIES ]]; do
            echo "Enter generator arguments (space-separated, no duplicates):"
            read -r -a args
            # Validate duplicates
            local dup=false
            for arg in "${args[@]}"; do
                count=0
                for a in "${args[@]}"; do [[ "$a" == "$arg" ]] && ((count++)); done
                if [[ $count -gt 1 ]]; then
                    dup=true
                    echo "Duplicate argument: $arg"
                fi
            done
            if $dup; then
                ((tries++))
                continue
            fi
            GEN_ARGS=("${args[@]}")
            break
        done
        if [[ $tries -eq $MAX_TRIES ]]; then
            echo "Failed to provide valid generator arguments after $MAX_TRIES tries."
            exit 1
        fi
        # Determine output file
        ITCH_FILE="$DATA_DIR/default.itch"
    fi
}

#==============================#
# Function: prompt_for_itch_file
# @brief    Prompt user to specify an existing .itch file
#==============================#
prompt_for_itch_file() {
    if [[ "$GEN_RUN" == true ]]; then
        echo "Generator will produce $ITCH_FILE"
        return
    fi
    local tries=0
    while [[ $tries -lt $MAX_TRIES ]]; do
        echo "Enter path to existing .itch file:"
        read -r file
        if [[ ! -f "$file" ]]; then
            echo "File does not exist."
            ((tries++))
            continue
        fi
        if [[ "$file" != *.itch ]]; then
            echo "File must end with .itch"
            ((tries++))
            continue
        fi
        ITCH_FILE="$file"
        break
    done
    if [[ $tries -eq $MAX_TRIES ]]; then
        echo "Failed to provide valid .itch file after $MAX_TRIES tries."
        exit 1
    fi
}

#==============================#
# Function: run_generator
# @brief    Run generator if requested
#==============================#
run_generator() {
    if [[ "$GEN_RUN" == false ]]; then return; fi
    mkdir -p "$DATA_DIR"
    echo "Running generator with arguments: ${GEN_ARGS[*]}"
    "$BIN_DIR/nsdq_generator" "${GEN_ARGS[@]}" --output "$ITCH_FILE"
    echo "Generator completed: $ITCH_FILE"
}

#==============================#
# Function: start_listener
# @brief    Start listener in background
#==============================#
start_listener() {
    LISTENER_BIN="$BIN_DIR/nsdq_listener"
    if [[ ! -x "$LISTENER_BIN" ]]; then
        echo "Error: Listener binary not found at $LISTENER_BIN"
        exit 1
    fi
    echo "Starting listener..."
    "$LISTENER_BIN" &
    LISTENER_PID=$!
    sleep 1 # Give time to start
}

#==============================#
# Function: start_replayer
# @brief    Start replayer and load messages
#==============================#
start_replayer() {
    REPLAYER_BIN="$BIN_DIR/nsdq_replayer"
    if [[ ! -x "$REPLAYER_BIN" ]]; then
        echo "Error: Replayer binary not found at $REPLAYER_BIN"
        kill $LISTENER_PID
        exit 1
    fi
    echo "Loading messages from $ITCH_FILE into memory..."
    "$REPLAYER_BIN" --file "$ITCH_FILE"
    echo "Replayer finished."
}

#==============================#
# Function: shutdown
# @brief    Cleanly stop listener
#==============================#
shutdown() {
    if ps -p $LISTENER_PID > /dev/null; then
        kill $LISTENER_PID
        wait $LISTENER_PID 2>/dev/null
    fi
}

#==============================#
# Function: print_report
# @brief    Print simple ASCII report
#==============================#
print_report() {
    local count
    count=$(wc -l < "$ITCH_FILE")
    echo "==================== REPORT ===================="
    echo "Date/Time  : $(date)"
    echo "File       : $ITCH_FILE"
    echo "Messages   : $count"
    echo "================================================"
}

#==============================#
# Main execution
#==============================#
prompt_for_generator
prompt_for_itch_file
run_generator
start_listener
start_replayer
shutdown
print_report

