import time
from multiprocessing import Process, Queue

def writer(q, num_messages):
    for _ in range(num_messages):
        q.put((101, 100.5, 10))

def reader(q, num_messages):
    start_time = time.time()
    total_read = 0
    last_print = 0
    
    for _ in range(num_messages):
        _ = q.get()
        total_read += 1
        
        # Calculate speed every 250,000 messages
        if total_read - last_print >= 250_000:
            current_time = time.time()
            elapsed = current_time - start_time
            if elapsed > 0:
                rate = (total_read - last_print) / elapsed
                # flush=True forces Python to bypass output buffering
                print(f"Active Rate: {rate:,.0f} msgs/sec", flush=True)
            last_print = total_read
            start_time = current_time

if __name__ == '__main__':
    # Give it enough messages to run for a few seconds
    NUM_MESSAGES = 5_000_000 
    q = Queue()
    
    p_writer = Process(target=writer, args=(q, NUM_MESSAGES))
    p_reader = Process(target=reader, args=(q, NUM_MESSAGES))
    
    p_writer.start()
    p_reader.start()
    
    p_writer.join()
    p_reader.join()