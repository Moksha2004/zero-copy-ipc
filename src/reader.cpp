#include "zcp_ipc.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <thread>

int main() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) { std::cerr << "Run writer first!\n"; return 1; }

    auto* layout = static_cast<SharedMemoryLayout*>(
        mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    );

    std::cout << "[Reader] Connected. Reading...\n";

    uint64_t processed = 0;
    while (true) {
        uint64_t tail = layout->control.tail.load(std::memory_order_relaxed);
        uint64_t head = layout->control.head.load(std::memory_order_acquire);

        if (tail < head) {
            size_t idx = tail & RING_BUFFER_MASK;
            
            volatile double price = layout->buffer[idx].price;
            layout->control.tail.store(tail + 1, std::memory_order_release);
            
            processed++;
            if (processed % 10000000 == 0) std::cout << "Read 10M messages. Last Price: " << price << "\n";
        } else {
            std::this_thread::yield();
        }
    }
}