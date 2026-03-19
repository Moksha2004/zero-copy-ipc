// #include "zcp_ipc.hpp"
// #include <iostream>
// #include <sys/mman.h>
// #include <fcntl.h>
// #include <thread>
// #include <chrono>

// int main() {
//     int fd = shm_open(SHM_NAME, O_RDWR, 0666);
//     if (fd == -1) { std::cerr << "Run writer first!\n"; return 1; }

//     auto* layout = static_cast<SharedMemoryLayout*>(
//         mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
//     );

//     std::cout << "[Reader] Connected. Reading...\n";

//     uint64_t processed = 0;
//     auto start_time = std::chrono::high_resolution_clock::now();
//     while (true) {
//         uint64_t tail = layout->control.tail.load(std::memory_order_relaxed);
//         uint64_t head = layout->control.head.load(std::memory_order_acquire);

//         if (tail < head) {
//             uint64_t count = head - tail;
//             processed += count;
            
//             // Move tail instantly to the head
//             layout->control.tail.store(head, std::memory_order_release);

//             if (processed >= 50000000) { 
//                 // Calculate every 50M messages
//                 auto now = std::chrono::high_resolution_clock::now();
//                 std::chrono::duration<double> elapsed = now - start_time;
                
//                 double rate = processed / elapsed.count();
//                 std::cout << "Active Rate: " << (uint64_t)rate << " msgs/sec"<<std::endl;
                
//                 processed = 0;
//                 start_time = std::chrono::high_resolution_clock::now();
//             }
//         } 
//         //For 100% cpu utilization
//     }
// }


#include "zcp_ipc.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <chrono>

#if defined(__x86_64__) || defined(_M_X64)
    #include <immintrin.h>
#endif

int main() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) { std::cerr << "Run writer first!\n"; return 1; }

    auto* layout = static_cast<SharedMemoryLayout*>(
        mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
    );

    std::cout << "[Reader] Pure C++ Speedometer (Batched Proof) Online..." << std::endl;

    uint64_t processed = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t tail = layout->control.tail.load(std::memory_order_relaxed);

    while (true) {
        uint64_t head = layout->control.head.load(std::memory_order_acquire);

        if (tail < head) {
            uint64_t count = head - tail;
            processed += count;
            tail = head; 

            if (processed >= 50000000) { 
                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = now - start_time;
                
                double rate = processed / elapsed.count();
                std::cout << "Active Rate: " << (uint64_t)rate << " msgs/sec" << std::endl;
                
                processed = 0;
                start_time = std::chrono::high_resolution_clock::now();
            }

            // THE PROOF: Artificial Python Simulator (Backoff)
            // We force C++ to waste time here. This stops it from spamming the memory bus, 
            // giving the Writer exclusive ownership of the L1 cache to blast a massive batch.
            for(int i = 0; i < 2000; i++) { 
                #if defined(__x86_64__) || defined(_M_X64)
                    _mm_pause(); 
                #endif
            }
            
        } else {
            // If there's no data, wait a bit before checking again
            for(int i = 0; i < 50; i++) { 
                #if defined(__x86_64__) || defined(_M_X64)
                    _mm_pause(); 
                #endif
            }
        }
    }
}