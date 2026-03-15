# # import zpc
# # import numpy as np
# # import time

# # def main():
# #     print("Connecting to Shared Memory...")
# #     try:
# #         reader = zpc.Reader()
# #     except Exception as e:
# #         print(f"Error: {e}")
# #         return

# #     # ZERO-COPY: Get the view once
# #     # We do NOT call this inside the loop. That would be slow.
# #     buffer = reader.get_buffer()
    
# #     print("Attached! Reading stream...")
    
# #     last_head = 0
# #     total_read = 0
# #     start_time = time.time()

# #     while True:
# #         # 1. Ask C++ where the writer is
# #         head = reader.get_head_pos()
        
# #         # 2. If new data arrived...
# #         if head > last_head:
# #             # Calculate how many new items (simple version)
# #             # In a real engine, we would handle the ring buffer wrap-around carefully
# #             # here we just grab the latest one to show speed.
            
# #             # Read directly from RAM (No copying!)
# #             # We use (head - 1) because head points to the NEXT empty slot
# #             idx = (head - 1) % 1024
# #             data = buffer[idx]
            
# #             total_read += (head - last_head)
# #             last_head = head

# #             if total_read % 100 == 0:
# #                 now = time.time()
# #                 elapsed = now - start_time
# #                 print(f"Read {total_read} msgs. Rate: {total_read/elapsed:.0f} msgs/sec. Latest Price: {data['price']:.2f}")

# # if __name__ == "__main__":
# #     main()


# # import zpc
# # import numpy as np
# # import time

# # def main():
# #     print("Connecting to Shared Memory...")
# #     try:
# #         reader = zpc.Reader()
# #     except Exception as e:
# #         print(f"Error: {e}")
# #         return

# #     # ZERO-COPY: Get the view once
# #     buffer = reader.get_buffer()
    
# #     print("Attached! Acting as CONSUMER (I will clear the buffer).")
    
# #     # We maintain our own local tail to know where we are reading
# #     # But we also need to tell C++ where we are
# #     current_tail = reader.get_tail_pos()
# #     total_read = 0
# #     start_time = time.time()

# #     while True:
# #         # 1. Check where the Writer is (Head)
# #         head = reader.get_head_pos()
        
# #         # 2. If there is new data (Head > Tail)
# #         if head > current_tail:
# #             # We have (head - current_tail) new items
# #             # In a real engine, we would process them here.
            
# #             # Let's read the latest one just to verify
# #             latest_idx = (head - 1) % 1024
# #             data = buffer[latest_idx]

# #             count = head - current_tail
# #             total_read += count
            
# #             # 3. CRITICAL: Move the Tail forward!
# #             # This tells the C++ Writer: "I am done with these slots. You can overwrite them."
# #             current_tail = head
# #             reader.update_tail(current_tail)

# #             if total_read % 1000000 == 0:
# #                 now = time.time()
# #                 elapsed = now - start_time
# #                 if elapsed > 0:
# #                     print(f"Processed {total_read} msgs. Rate: {total_read/elapsed:.0f} msgs/sec. Price: {data['price']:.2f}")

# # if __name__ == "__main__":
# #     main()


# # import zpc
# # import time

# # def main():
# #     print("Connecting to Shared Memory...")
# #     try:
# #         reader = zpc.Reader()
# #         buffer = reader.get_buffer()
# #     except Exception as e:
# #         print(f"Error: {e}")
# #         return

# #     print("Attached! Strategy Engine Online. Waiting for data...")
    
# #     current_tail = reader.get_tail_pos()
# #     prices = [] # Store prices for our Moving Average

# #     start_time = time.time()
# #     total_read = 0
# #     last_print = 0

# #     while True:
# #         head = reader.get_head_pos()
        
# #         if head > current_tail:
# #             count = head - current_tail
# #             total_read += count
            
# #             for i in range(current_tail, head):
# #                 idx = i % 1024
# #                 data = buffer[idx]
# #                 prices.append(data['price'])
# #                 if len(prices) > 50:
# #                     prices.pop(0)

# #             # Strategy Logic
# #             if len(prices) >= 10:
# #                 sma = sum(prices) / len(prices)
# #                 current_price = data['price']
                
# #                 signal = "HOLD"
# #                 if current_price > sma + 0.5: signal = "SELL"
# #                 elif current_price < sma - 0.5: signal = "BUY"
                
# #                 # --- NEW SPEEDOMETER LOGIC ---
# #                 if total_read - last_print >= 50000:
# #                     elapsed = time.time() - start_time
# #                     rate = total_read / elapsed
# #                     print(f"Rate: {rate:,.0f} msgs/sec | Price: {current_price:.2f} | Signal: {signal}")
# #                     last_print = total_read

# #             current_tail = head
# #             reader.update_tail(current_tail)
            
# #         # time.sleep(0.001) # You can remove this sleep too for max speed

# # if __name__ == "__main__":
# #     main()




# # import zpc
# # import time

# # def main():
# #     print("Connecting to Shared Memory...")
# #     try:
# #         reader = zpc.Reader()
# #         buffer = reader.get_buffer() # The Zero-Copy NumPy View
# #     except Exception as e:
# #         print(f"Error: {e}")
# #         return

# #     print("Attached! Speedometer Online. Measuring throughput...")
    
# #     current_tail = reader.get_tail_pos()
    
# #     start_time = time.time()
# #     total_read = 0
# #     last_print = 0

# #     while True:
# #         head = reader.get_head_pos()
        
# #         if head > current_tail:
# #             # Calculate how many messages we bypassed in a single jump
# #             count = head - current_tail
# #             total_read += count
            
# #             # Print the speed every 50 Million messages
# #             if total_read - last_print >= 50_000_000:
# #                 elapsed = time.time() - start_time
# #                 rate = total_read / elapsed
                
# #                 # Grab the absolute latest price to prove we are reading it
# #                 latest_idx = (head - 1) % 1024
# #                 latest_price = buffer[latest_idx]['price']
                
# #                 print(f"Rate: {rate:,.0f} msgs/sec | Total: {total_read:,} | Latest Price: {latest_price:.2f}")
# #                 last_print = total_read

# #             # Tell C++ we are done
# #             current_tail = head
# #             reader.update_tail(current_tail)

# # if __name__ == "__main__":
# #     main()

# import zpc
# import time
# import numpy as np
# def main():
#     print("Connecting to Shared Memory...")
#     try:
#         reader = zpc.Reader()
#         buffer = reader.get_buffer()
#     except Exception as e:
#         print(f"Error: {e}")
#         return

#     print("Attached! Active Consumer Online. Measuring application throughput...")
    
#     current_tail = reader.get_tail_pos()
    
#     start_time = time.time()
#     total_read = 0
#     last_print = 0
#     np_buffer = np.array(buffer, copy=False)
#     while True:
#         head = reader.get_head_pos()
        
#         # --- THE OVERRUN CHECK (The fix for your exact observation) ---
#         if head - current_tail > 1024:
#             # C++ lapped us. The old data is physically gone. 
#             # Snap to the oldest surviving message currently in the buffer.
#             current_tail = head - 1024
            
#         if head > current_tail:
#             count = head - current_tail
#             total_read += count
            
#             all_prices = np_buffer['price'] 
            
#             # Grab the latest price for the print statement
#             latest_idx = (head - 1) % 1024
#             price = all_prices[latest_idx]

#             # Calculate speed every 10 Million messages
#             if total_read - last_print >= 10_000_000:
#                 current_time = time.time()
#                 elapsed = current_time - start_time
#                 rate = (total_read - last_print) / elapsed
                
#                 print(f"Active Rate: {rate:,.0f} msgs/sec | Total: {total_read:,} | Latest Price: {price:.2f}")
                
#                 last_print = total_read
#                 start_time = current_time

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
    
    # Zero-Copy NumPy View
    np_buffer = np.array(buffer, copy=False)
    
    while True:
        head = reader.get_head_pos()

        # Overrun Check: Drop stale data if C++ lapped the Python consumer
        if head - current_tail > 1024:
            current_tail = head - 1024

        if head > current_tail:
            count = head - current_tail
            total_read += count

            # Vectorized extraction (Bypasses Python for-loop bottleneck)
            all_prices = np_buffer['price']
            latest_idx = (head - 1) % 1024
            price = all_prices[latest_idx]

            # Calculate and print instantaneous throughput
            if total_read - last_print >= 10_000_000:
                current_time = time.time()
                elapsed = current_time - start_time
                
                if elapsed > 0:
                    rate = (total_read - last_print) / elapsed
                    print(f"Active Rate: {rate:,.0f} msgs/sec | Total: {total_read:,} | Latest Price: {price:.2f}")

                last_print = total_read
                start_time = current_time

            # Commit the read
            current_tail = head
            reader.update_tail(current_tail)

if __name__ == "__main__":
    main()
