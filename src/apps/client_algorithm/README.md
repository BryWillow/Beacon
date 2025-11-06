# Client Algorithm - Market Data Test Receiver

Simple C++ client for testing market data reception from the playback application.

## Features

- **TCP Mode**: Connects to TCP server and receives stream
- **UDP Multicast Mode**: Joins multicast group and receives datagrams
- Protocol-agnostic (works with NASDAQ, NYSE, CME formats)
- Progress tracking and statistics
- Hex dump of first message for verification

## Building

```bash
./build.sh
```

Binary output: `build/client_algorithm`

## Usage

### TCP Mode (Recommended for Testing)

```bash
# Connect to localhost:12345 (default)
./build/client_algorithm tcp

# Connect to custom host/port
./build/client_algorithm tcp 192.168.1.100 54321
```

### UDP Multicast Mode

```bash
# Listen on 239.255.0.1:12345 (default)
./build/client_algorithm udp

# Listen on custom multicast address/port
./build/client_algorithm udp 239.255.1.100 54321
```

## Testing with Playback App

### TCP Testing (Recommended - Always Works)

```bash
# Terminal 1: Start playback server (waits for client)
cd ../exchange_market_data_playback
./build/exchange_market_data_playback --config config_tcp.json output.mdp

# Terminal 2: Start client
cd ../client_algorithm
./build/client_algorithm tcp 127.0.0.1 12345
```

### UDP Testing (May Not Work on Localhost)

```bash
# Terminal 1: Start client first
cd ../client_algorithm
./build/client_algorithm udp 239.255.0.1 12345

# Terminal 2: Start playback
cd ../exchange_market_data_playback
./build/exchange_market_data_playback --config config_udp_slow.json output.mdp
```

**Note**: UDP multicast on localhost may not work on macOS due to OS routing limitations. Use TCP mode for reliable localhost testing.

## Example Output

### TCP Mode
```
=============================================================================
          TCP MARKET DATA CLIENT - TEST RECEIVER                            
=============================================================================
Mode: TCP
Connecting to: 127.0.0.1:12345

Connecting...
✓ Connected successfully!

Receiving market data (Press Ctrl+C to stop)...
=============================================================================

First message (first 16 bytes): 01 00 00 00 e8 03 00 00 4d 53 46 54 00 00 00 00 

Received 1000 messages (32000 bytes)
Received 2000 messages (64000 bytes)
Received 3000 messages (96000 bytes)
...
Received 20000 messages (640000 bytes)

✓ Server closed connection (playback complete)

=============================================================================
                         RECEPTION COMPLETE                                 
=============================================================================
Total messages: 20000
Total bytes:    640000
=============================================================================
```

### UDP Mode
```
=============================================================================
          UDP MARKET DATA CLIENT - TEST RECEIVER                            
=============================================================================
Mode: UDP Multicast
Listening on: 239.255.0.1:12345

✓ Joined multicast group successfully!

Receiving market data (Press Ctrl+C to stop)...
=============================================================================

First message from 192.168.1.50:12345
Message size: 32 bytes
First 16 bytes: 01 00 00 00 e8 03 00 00 4d 53 46 54 00 00 00 00 

Received 1000 messages (32000 bytes)
Received 2000 messages (64000 bytes)
...
```

## Message Format

All market data messages are **32 bytes** in binary format:

- Bytes 0-3: Message type
- Bytes 4-7: Timestamp
- Bytes 8-15: Symbol (8 chars, null-padded)
- Bytes 16-31: Protocol-specific data

The client is protocol-agnostic and just receives raw binary data.

## Troubleshooting

### TCP: Connection refused
**Problem**: Client can't connect to server

**Solution**: Make sure playback app is running in TCP mode first:
```bash
cd ../exchange_market_data_playback
./build/exchange_market_data_playback --config config_tcp.json output.mdp
```

### UDP: No messages received
**Problem**: Client receives 0 messages on localhost

**Cause**: macOS multicast loopback limitation

**Solution**: Use TCP mode instead, or test on separate machines

### Partial messages received
**Problem**: Byte count doesn't match message count × 32

**Check**: 
- TCP should always receive complete messages
- UDP may have packet loss (check playback config for `packet_loss_rate`)

---

**Project**: Beacon  
**Application**: client_algorithm  
**Author**: Bryan Camp
