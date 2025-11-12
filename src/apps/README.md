# Beacon Apps

## Overview

This directory contains the main applications for the Beacon trading system.  
Each app is modular, focused, and uses true exchange data for realistic simulation and testing.

---

## App Order & Purpose

1. **generator/**  
   Generates binary market data files from true exchange data (NSDQ, NYSE, CME).

2. **playback/**  
   Replays binary market data files with configurable timing, burst, wave, and chaos modes.  
   Modular configuration via JSON for senders, authorities (rules), and advisors (classifiers).

3. **algorithm/**  
   Contains your trading algorithms and strategies.  
   Connects to playback and matching engine for live simulation.

4. **matching_engine/**  
   Implements the order matching logic using the OUCH protocol and lock-free data structures.

---

## How They Work Together

- **generator** creates realistic market data files.
- **playback** streams these files to your algorithms and matching engine.
- **algorithm** receives market data, makes trading decisions, and sends orders.
- **matching_engine** processes orders, matches trades, and updates state.

---

## Usage

See each app's README for details and usage instructions.

---

## License

MIT

---

**Built for modular, high-performance trading system development**
