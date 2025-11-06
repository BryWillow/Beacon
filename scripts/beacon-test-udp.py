#!/usr/bin/env python3
"""
Simple UDP multicast listener for testing market data playback
Usage: python3 udp_listener.py [multicast_address] [port]
"""

import socket
import struct
import sys

def listen_multicast(multicast_group='239.255.0.1', port=12345):
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Increase receive buffer size for high-frequency data
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 2 * 1024 * 1024)  # 2 MB
    
    # Bind to the port
    sock.bind(('', port))
    
    # Tell the kernel to add the socket to the multicast group
    mreq = struct.pack("4sl", socket.inet_aton(multicast_group), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    
    print(f"Listening on {multicast_group}:{port}")
    print(f"Waiting for packets... (Press Ctrl+C to stop)\n")
    
    message_count = 0
    total_bytes = 0
    
    try:
        while True:
            data, address = sock.recvfrom(1024)
            message_count += 1
            total_bytes += len(data)
            
            if message_count % 1000 == 0:
                print(f"Received {message_count} messages ({total_bytes} bytes)")
                
    except KeyboardInterrupt:
        print(f"\n\nTotal messages received: {message_count}")
        print(f"Total bytes received: {total_bytes}")
        sock.close()

if __name__ == '__main__':
    multicast_addr = sys.argv[1] if len(sys.argv) > 1 else '239.255.0.1'
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 12345
    listen_multicast(multicast_addr, port)
