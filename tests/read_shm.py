import zpc
import numpy as np
import time

def main():
    print("Connecting to Shared Memory...")
    try:
        reader = zpc.Reader()
    except Exception as e:
        print(f"Error: {e}")
        return

    # ZERO-COPY: Get the view once
    # We do NOT call this inside the loop. That would be slow.
    buffer = reader.get_buffer()
    
    print("Attached! Reading stream...")
    
    last_head = 0
    total_read = 0
    start_time = time.time()

    while True:
        # 1. Ask C++ where the writer is
        head = reader.get_head_pos()
        
        # 2. If new data arrived...
        if head > last_head:
            # Calculate how many new items (simple version)
            # In a real engine, we would handle the ring buffer wrap-around carefully
            # here we just grab the latest one to show speed.
            
            # Read directly from RAM (No copying!)
            # We use (head - 1) because head points to the NEXT empty slot
            idx = (head - 1) % 1024
            data = buffer[idx]
            
            total_read += (head - last_head)
            last_head = head

            if total_read % 100 == 0:
                now = time.time()
                elapsed = now - start_time
                print(f"Read {total_read} msgs. Rate: {total_read/elapsed:.0f} msgs/sec. Latest Price: {data['price']:.2f}")

if __name__ == "__main__":
    main()