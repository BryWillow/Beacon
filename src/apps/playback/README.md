# Exchange Market Data Playback

## Overview

This app replays binary market data files (NSDQ, NYSE, CME) with configurable timing, burst, wave, and chaos modes.  
It is highly modular: senders, playback authorities (rules), and advisors (classifiers) are all configured via JSON files.

---

## How the Parts Work Together

- **main.cpp** is the entry point. It loads configuration, sets up the sender, rules, and advisors, and runs playback.
- **market_data_playback.h** contains the playback engine, which reads messages from binary files and applies rules/advisors before sending.
- **message_senders/** provides different sender types (UDP, TCP, Console). The sender is chosen via config.
- **playback_authorities/** contains rules (authorities) that control message flow, timing, and chaos features.
- **playback_advisors/** contains classifiers (advisors) that analyze messages and provide priority or burst detection.
- **Config files** in `config/playback/` define which sender, rules, and advisors to use. The main config is `default.json`, which references all others.

**Typical Flow:**
1. `main.cpp` loads `default.json`.
2. It loads referenced sender, authority, and advisor configs.
3. It instantiates the sender, rules, and advisors.
4. The playback engine reads the binary file, applies rules/advisors, and sends messages via the chosen sender.

---

## Configuration

- All configs are in `config/playback/`.
- `default.json` references sender, advisor, and authority configs.
- Example sender config: `senders/sender_udp.json`
- Example authority config: `authorities/authority_burst.json`
- Example advisor config: `advisors/advisor_symbol_priority.json`

---

## Modes Supported

- **Continuous:** Normal rate playback
- **Burst:** Bursts of messages at intervals
- **Wave:** Variable rate playback
- **Chaos:** Packet loss, jitter, etc.

---

## Usage

```bash
./exchange_market_data_playback --config config/playback/default.json <input_file>
./exchange_market_data_playback --summary <input_file>
```
- `--summary` prints the number of messages sent every 5 seconds and at exit.

---

## Extending

- Add new sender, advisor, or authority configs and update `default.json`.
- Implement new rules or classifiers in their respective directories.
- The app will automatically use new configs if referenced in `default.json`.

---

## Testing

- Tests are organized by purpose (see `tests/`).
- Binary file parsing, config parsing, UDP/TCP/Console sending, and all modes are covered.

---

## License

MIT

---

## Examples: Creating Custom Modes

You can create custom playback modes by adding new authority (rule) config files and referencing them in `default.json`.

### Example 1: "Super Burst" Mode

Create `config/playback/authorities/authority_super_burst.json`:
```json
{
  "type": "burst_rule",
  "burst_size": 10000,
  "burst_interval_ms": 20
}
```
Add to `"authorities"` in `default.json`:
```json
"authorities": [
  ...existing code...,
  "authorities/authority_super_burst.json"
]
```

### Example 2: "Ultra Chaos" Mode

Create `config/playback/authorities/authority_ultra_chaos.json`:
```json
{
  "type": "chaos_rule",
  "drop_probability": 0.05,
  "max_jitter_us": 5000
}
```
Add to `"authorities"` in `default.json`:
```json
"authorities": [
  ...existing code...,
  "authorities/authority_ultra_chaos.json"
]
```

### Example 3: "Wave + Burst" Hybrid Mode

Create `config/playback/authorities/authority_wave_burst.json`:
```json
{
  "type": "wave_rule",
  "period_ms": 5000,
  "min_rate": 500,
  "max_rate": 20000,
  "speed_factor": 1.5
}
```
Create `config/playback/authorities/authority_burst.json` (if not already present):
```json
{
  "type": "burst_rule",
  "burst_size": 500,
  "burst_interval_ms": 50
}
```
Reference both in `default.json`:
```json
"authorities": [
  "authorities/authority_wave_burst.json",
  "authorities/authority_burst.json"
]
```

---

**Tip:**  
You can combine multiple authority configs for complex playback behavior.  
Just create a new JSON file for each mode and add its path to the `"authorities"` array in `default.json`.

---

**Built for robust, configurable market data playback**
