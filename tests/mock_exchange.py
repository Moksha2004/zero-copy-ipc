import socket
import struct
import time
import random

def main():
    print("Starting Mock Exchange on 127.0.0.1:8080...")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    symbol_id = 101
    price = 100.0
    
    try:
        while True:
            price += random.uniform(-1.0, 1.0)
            volume = random.randint(10, 100)
            
            # THE FIX: Add 4 bytes of padding before the double, and 4 at the end
            # I = 4 bytes (int)
            # 4x = 4 bytes (empty padding)
            # d = 8 bytes (double)
            # I = 4 bytes (int)
            # 4x = 4 bytes (empty padding)
            packet = struct.pack('<I 4x d I 4x', symbol_id, price, volume)
            
            sock.sendto(packet, ("127.0.0.1", 8080))
            # time.sleep(0.05) 
            
    except KeyboardInterrupt:
        print("\nExchange Shutting Down.")

if __name__ == "__main__":
    main()