# Playback Rules Engine Architecture

## Message Flow

```
┌─────────────────┐
│ Binary File     │  output.mdp (10,000 messages × 32 bytes)
│ (on disk)       │
└────────┬────────┘
         │ Load entire file at startup
         ▼
┌─────────────────┐
│ MessageBuffer   │  All messages in memory (fast, no I/O delays)
│ (in memory)     │
└────────┬────────┘
         │ For each message...
         ▼
┌─────────────────────────────────────────┐
│         RULES ENGINE                    │
│  (evaluates rules in priority order)    │
└─────────────────────────────────────────┘
         │
         ├─────────────────────────────┐
         │                             │
         ▼                             ▼
┌──────────────────┐          ┌──────────────────┐
│ SAFETY RULES     │          │ Message enters    │
│ (Priority 0)     │          │ with Decision{}   │
│                  │          │                   │
│ - Rate Limit     │──────▶   │ decision.outcome  │
│   "Never exceed  │          │ decision.delay    │
│    100K msgs/sec"│          └──────────┬────────┘
│                  │                     │
│ Can VETO sending │                     │ Pass or VETO?
└──────────────────┘                     │
         │                               ▼
         │                     ┌──────────────────┐
         ▼                     │ If VETO:         │
┌──────────────────┐           │   STOP HERE      │
│ CONTROL RULES    │           │   (queued)       │
│ (Priority 1)     │           └──────────────────┘
│                  │                     │
│ - Burst Pattern  │                     │ Otherwise continue
│   "Send 5000,    │                     ▼
│    wait 100ms"   │          ┌──────────────────┐
│                  │──────▶   │ Apply burst logic│
│ - Continuous     │          │ Add/clear delays │
│   "Send at 10K   │          └──────────┬────────┘
│    msgs/sec"     │                     │
└──────────────────┘                     │
         │                               ▼
         │                     ┌──────────────────┐
         ▼                     │ decision.delay   │
┌──────────────────┐           │ may be set to 0  │
│ TIMING RULES     │           │ or increased     │
│ (Priority 2)     │           └──────────┬────────┘
│                  │                      │
│ - Speed Factor   │                      ▼
│   "2x faster"    │──────▶    ┌──────────────────┐
│   Scales all     │           │ Scale delay by   │
│   delays         │           │ speed factor     │
│                  │           │ delay = delay/2.0│
└──────────────────┘           └──────────┬────────┘
         │                                 │
         │                                 ▼
         ▼                      ┌──────────────────┐
┌──────────────────┐            │ decision.delay   │
│ CHAOS RULES      │            │ now scaled       │
│ (Priority 3)     │            └──────────┬────────┘
│                  │                       │
│ - Packet Loss    │                       ▼
│   "Drop 1%"      │            ┌──────────────────┐
│                  │──────▶     │ Random drop?     │
│ - Jitter         │            │ If yes: DROP     │
│   "±1ms random"  │            │ If no: continue  │
└──────────────────┘            └──────────┬────────┘
         │                                  │
         │                                  ▼
         │                       ┌──────────────────┐
         │                       │ Final Decision   │
         │                       │ • SEND_NOW       │
         │                       │ • DROP           │
         │                       │ • VETO (queued)  │
         │                       └──────────┬────────┘
         │                                  │
         ▼                                  ▼
┌─────────────────────────────────────────────────┐
│           PLAYBACK ORCHESTRATOR                 │
│                                                 │
│  if (decision.delay > 0):                      │
│      sleep(decision.delay)                     │
│                                                 │
│  if (decision.outcome == SEND_NOW):            │
│      sender.send(message)                      │
│      state.recordSent()                        │
│                                                 │
│  if (decision.outcome == DROP):                │
│      state.recordDropped()                     │
│                                                 │
│  if (decision.outcome == VETO):                │
│      state.recordQueued()                      │
└─────────────────┬───────────────────────────────┘
                  │
                  ▼
         ┌─────────────────┐
         │ Message Sender  │
         │                 │
         │ - Console       │  (current)
         │ - UDP Multicast │  (future)
         │ - TCP Socket    │  (future)
         │ - File Output   │  (future)
         └─────────────────┘
```

## Example: Burst + Speed + Rate Limit

### Configuration
```json
{
  "mode": "burst",
  "burst_size": 5000,
  "burst_interval_ms": 100,
  "speed_factor": 2.0,
  "max_rate_limit": 100000
}
```

### Message #1 Flow
```
Message #1 enters
    ↓
[SAFETY] Rate Limit: currentRate=0, OK
    ↓
[CONTROL] Burst: messagesInBurst=0 < 5000, SEND_NOW, delay=0
    ↓
[TIMING] Speed Factor: delay=0, no change
    ↓
Decision: SEND_NOW, delay=0
    ↓
Send immediately
```

### Message #5001 Flow
```
Message #5001 enters
    ↓
[SAFETY] Rate Limit: currentRate=50000, OK
    ↓
[CONTROL] Burst: messagesInBurst=5001 > 5000
             elapsed=50ms < 100ms
             ADD DELAY: 50ms (wait for next burst)
    ↓
[TIMING] Speed Factor: delay=50ms / 2.0 = 25ms
    ↓
Decision: MODIFIED, delay=25ms
    ↓
Sleep 25ms, then send
```

### Message #1 with Rate Exceeded
```
Message #1 enters
    ↓
[SAFETY] Rate Limit: currentRate=105000 > 100000
             VETO + add backoff delay
    ↓
Decision: VETO
    ↓
Message queued (not sent)
```

## Key Takeaways

1. **Rules are independent**: Each rule only cares about its own logic
2. **Priority matters**: Higher priority rules can block lower priority rules
3. **Delays accumulate**: Multiple rules can add delays (then scaled by speed factor)
4. **VETO is terminal**: Safety rules can stop message processing immediately
5. **Protocol-agnostic**: No parsing needed, just forward 32-byte chunks
