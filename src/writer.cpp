#include "zcp_ipc.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>

int main() {
	int fd=shm_open(SHM_NAME,O_CREAT | O_RDWR, 0666);
	ftruncate(fd, sizeof(SharedMemoryLayout));
    
    // 2. Map into RAM
    auto* layout = static_cast<SharedMemoryLayout*>(
        mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    );

    std::cout << "[Writer] Started. Sending data...\n";

    uint64_t count = 0;
    while (true) {
        uint64_t head = layout->control.head.load(std::memory_order_relaxed);
        uint64_t tail = layout->control.tail.load(std::memory_order_acquire);

        if (head - tail < RING_BUFFER_SIZE) {
            size_t idx = head & RING_BUFFER_MASK;

            layout->buffer[idx] = {101, 100.0 + (count * 0.01), 50};

            layout->control.head.store(head + 1, std::memory_order_release);
            
            count++;
            if (count % 10000000 == 0) std::cout << "Sent 10M messages\n";
        } else {
            std::this_thread::yield();
        }
    }
}

