# import zpc
# import numpy as np
# import time

# def main():
#     print("Connecting to Shared Memory...")
#     try:
#         reader = zpc.Reader()
#     except Exception as e:
#         print(f"Error: {e}")
#         return

#     # ZERO-COPY: Get the view once
#     # We do NOT call this inside the loop. That would be slow.
#     buffer = reader.get_buffer()
    
#     print("Attached! Reading stream...")
    
#     last_head = 0
#     total_read = 0
#     start_time = time.time()

#     while True:
#         # 1. Ask C++ where the writer is
#         head = reader.get_head_pos()
        
#         # 2. If new data arrived...
#         if head > last_head:
#             # Calculate how many new items (simple version)
#             # In a real engine, we would handle the ring buffer wrap-around carefully
#             # here we just grab the latest one to show speed.
            
#             # Read directly from RAM (No copying!)
#             # We use (head - 1) because head points to the NEXT empty slot
#             idx = (head - 1) % 1024
#             data = buffer[idx]
            
#             total_read += (head - last_head)
#             last_head = head

#             if total_read % 100 == 0:
#                 now = time.time()
#                 elapsed = now - start_time
#                 print(f"Read {total_read} msgs. Rate: {total_read/elapsed:.0f} msgs/sec. Latest Price: {data['price']:.2f}")

# if __name__ == "__main__":
#     main()


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
    buffer = reader.get_buffer()
    
    print("Attached! Acting as CONSUMER (I will clear the buffer).")
    
    # We maintain our own local tail to know where we are reading
    # But we also need to tell C++ where we are
    current_tail = reader.get_tail_pos()
    total_read = 0
    start_time = time.time()

    while True:
        # 1. Check where the Writer is (Head)
        head = reader.get_head_pos()
        
        # 2. If there is new data (Head > Tail)
        if head > current_tail:
            # We have (head - current_tail) new items
            # In a real engine, we would process them here.
            
            # Let's read the latest one just to verify
            latest_idx = (head - 1) % 1024
            data = buffer[latest_idx]

            count = head - current_tail
            total_read += count
            
            # 3. CRITICAL: Move the Tail forward!
            # This tells the C++ Writer: "I am done with these slots. You can overwrite them."
            current_tail = head
            reader.update_tail(current_tail)

            if total_read % 1000000 == 0:
                now = time.time()
                elapsed = now - start_time
                if elapsed > 0:
                    print(f"Processed {total_read} msgs. Rate: {total_read/elapsed:.0f} msgs/sec. Price: {data['price']:.2f}")

if __name__ == "__main__":
    main()