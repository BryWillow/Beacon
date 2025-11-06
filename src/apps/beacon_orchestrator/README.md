# Beacon Orchestrator

Professional process orchestration and monitoring for the Beacon Trading System.

## Overview

The Beacon Orchestrator is a Python-based process manager that:
- Coordinates startup of all system components
- Provides real-time progress monitoring
- Captures and displays final statistics
- Handles graceful shutdown on Ctrl+C
- Eliminates timing issues with bash scripts

## Usage

```bash
# Run with default 60 second duration
./beacon_orchestrator.py

# Run with custom duration
./beacon_orchestrator.py 30

# Or from the experimental root directory
python3 src/apps/beacon_orchestrator/beacon_orchestrator.py 30
```

## Features

✅ **Professional Output**
- Timestamped startup steps
- Step duration tracking
- Color-coded status messages
- Real-time progress bar

✅ **Reliable Statistics**
- Captures algorithm output directly
- No sleep-based timing issues
- Displays final stats immediately

✅ **Graceful Shutdown**
- Handles Ctrl+C cleanly
- Terminates all child processes
- Waits for clean exit

✅ **Process Management**
- Verifies all binaries exist
- Detects crashes early
- Reports failures clearly

## Architecture

The orchestrator spawns three child processes:
1. **Matching Engine** - OUCH order matching (TCP port 9000)
2. **Algorithm** - Your trading strategy (connects to matching engine, receives market data)
3. **Market Data Playback** - ITCH message replay (UDP multicast 239.255.0.1:12345)

All processes run independently with **zero performance impact**. The orchestrator only:
- Starts them
- Monitors their status
- Collects their output when they finish

## Requirements

- Python 3.6+
- All Beacon components built:
  - `exchange_matching_engine`
  - `algo_template`
  - `exchange_market_data_playback`
- Market data file: `src/apps/exchange_market_data_generator/output.itch`

## Comparison with Bash Script

| Feature | Bash Script | Python Orchestrator |
|---------|-------------|---------------------|
| Statistics Display | Unreliable (sleep-based) | Reliable (process communication) |
| Error Handling | Basic | Comprehensive |
| Process Monitoring | Manual PID tracking | Built-in subprocess management |
| Output Capture | File redirection + grep | Direct stdout capture |
| Code Maintainability | ~300 lines of shell | ~350 lines of clean Python |

## Why Python?

- **Subprocess Management**: Python's `subprocess` module is designed for this
- **No Timing Issues**: `communicate()` waits for process completion
- **Cross-Platform**: Works on macOS, Linux, Windows (with minor tweaks)
- **Readable**: Much easier to debug than bash
- **Zero Performance Impact**: Orchestrator is not in the hot path

## Future Enhancements

Potential improvements:
- [ ] Tail logs in real-time to separate files
- [ ] Web dashboard for monitoring
- [ ] Automatic retry on component failure
- [ ] Distributed mode (components on different machines)
- [ ] Performance profiling integration
