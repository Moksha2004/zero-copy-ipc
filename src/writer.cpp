
#include "zcp_ipc.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // 1. Initialize POSIX Shared Memory
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedMemoryLayout));
    
    auto* layout = static_cast<SharedMemoryLayout*>(
        mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    );

    std::cout << "[Writer] Zero-Copy Memory Engine Online.\n";
    std::cout << "[Writer] Generating data at MAXIMUM CPU speed...\n";

    Market data;
    data.symbol_id = 101;
    data.price = 100.0;
    data.volume = 10;

    uint64_t local_head = 0;

    // 2. Unrestricted Lock-Free Ingestion Loop
    while (true) {
        data.price += 0.01;
        data.volume++;

        uint32_t slot = local_head & RING_BUFFER_MASK; // Instead of mod ()
        layout->buffer[slot] = data;

        local_head++;
        // Memory barrier ensures data is physically written before head increments
        layout->control.head.store(local_head, std::memory_order_release);
    }

    return 0;
}