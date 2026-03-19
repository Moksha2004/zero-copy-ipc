

# import zpc
# import time
# import numpy as np

# def main():
#     print("Connecting to Shared Memory...")
#     try:
#         reader = zpc.Reader()
#         buffer = reader.get_buffer()
#     except Exception as e:
#         print(f"Error: {e}. Is the C++ writer running?")
#         return

#     print("Attached! Active Consumer Online. Measuring vectorized throughput...")

#     current_tail = reader.get_tail_pos()
#     start_time = time.time()
#     total_read = 0
#     last_print = 0
    
#     # NumPy View
#     np_buffer = np.array(buffer, copy=False)
    
#     while True:
#         head = reader.get_head_pos()

#         # Overrun Check: Drop stale data if C++ lapped the Python consumer
#         if head - current_tail > 1024:
#             current_tail = head - 1024

#         if head > current_tail:
#             count = head - current_tail
#             total_read += count

#             # Vectorized extraction (To avoid for-loop bottleneck)
#             all_prices = np_buffer['price']
#             latest_idx = (head - 1) % 1024
#             price = all_prices[latest_idx]

#             # Calculate and print instantaneous throughput
#             if total_read - last_print >= 10_000:
#                 current_time = time.time()
#                 elapsed = current_time - start_time
                
#                 if elapsed > 0:
#                     rate = (total_read - last_print) / elapsed
#                     print(f"Active Rate: {rate:,.0f} msgs/sec | Total: {total_read:,} | Latest Price: {price:.2f}",flush=True)

#                 last_print = total_read
#                 start_time = current_time

#             # Commit the read
#             current_tail = head
#             reader.update_tail(current_tail)

# if __name__ == "__main__":
#     main()


import zpc
import time
import numpy as np

def main():
    print("Connecting to Shared Memory...")
    try:
        reader = zpc.Reader()
        buffer = reader.get_buffer()
    except Exception as e:
        print(f"Error: {e}. Is the C++ writer running?")
        return

    print("Attached! Active Consumer Online. Measuring vectorized throughput...")

    current_tail = reader.get_tail_pos()
    start_time = time.time()
    total_read = 0
    last_print = 0
    
    # ZERO-COPY: Wrap the C++ memory in a NumPy array without copying it
    np_buffer = np.array(buffer, copy=False)
    
    while True:
        head = reader.get_head_pos()

        # THE OVERRUN CHECK: Drop stale data if C++ lapped the Python consumer
        if head - current_tail > 1024:
            current_tail = head - 1024

        if head > current_tail:
            count = head - current_tail
            total_read += count

            # VECTORIZATION: Extract data in bulk via C-backend, bypassing Python loops
            all_prices = np_buffer['price']
            
            # Grab the absolute latest price for the console output
            latest_idx = (head - 1) % 1024
            price = all_prices[latest_idx]

            # Calculate and print instantaneous throughput every 10M messages
            if total_read - last_print >= 10_000_000:
                current_time = time.time()
                elapsed = current_time - start_time
                
                if elapsed > 0:
                    rate = (total_read - last_print) / elapsed
                    # flush=True is critical to prevent OS buffering during automated benchmarks
                    print(f"Active Rate: {rate:,.0f} msgs/sec | Total: {total_read:,} | Latest Price: {price:.2f}", flush=True)

                last_print = total_read
                start_time = current_time

            # Commit the read to let C++ know these slots are free
            current_tail = head
            reader.update_tail(current_tail)

if __name__ == "__main__":
    main()
