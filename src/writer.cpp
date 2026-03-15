// // #include "zcp_ipc.hpp"
// // #include <iostream>
// // #include <sys/mman.h>
// // #include <fcntl.h>
// // #include <unistd.h>
// // #include <thread>
// // #include <chrono>
// // #include <netinet/in.h> // For Sockets
// // #include <sys/socket.h> // For Sockets

// // int main() {
// // 	int fd=shm_open(SHM_NAME,O_CREAT | O_RDWR, 0666);
// // 	ftruncate(fd, sizeof(SharedMemoryLayout));
    
// //     // 2. Map into RAM
// //     auto* layout = static_cast<SharedMemoryLayout*>(
// //         mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
// //     );

// //     std::cout << "[Writer] Started. Sending data...\n";

// //     uint64_t count = 0;
// //     while (true) {
// //         uint64_t head = layout->control.head.load(std::memory_order_relaxed);
// //         uint64_t tail = layout->control.tail.load(std::memory_order_acquire);

// //         if (head - tail < RING_BUFFER_SIZE) {
// //             size_t idx = head & RING_BUFFER_MASK;

// //             layout->buffer[idx] = {101, 100.0 + (count * 0.01), 50};

// //             layout->control.head.store(head + 1, std::memory_order_release);
            
// //             count++;
// //             if (count % 10000000 == 0) std::cout << "Sent 10M messages\n";
// //         } else {
// //             std::this_thread::yield();
// //         }
// //     }
// // }


// // #include "zcp_ipc.hpp"
// // #include <iostream>
// // #include <sys/mman.h>
// // #include <fcntl.h>
// // #include <unistd.h>
// // #include <sys/socket.h>  // NEW: For sockets
// // #include <netinet/in.h>  // NEW: For sockaddr_in

// // int main() {
// //     // 1. Setup Shared Memory (Your existing code)
// //     int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
// //     ftruncate(fd, sizeof(SharedMemoryLayout));
// //     auto* layout = static_cast<SharedMemoryLayout*>(
// //         mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
// //     );

// //     // 2. Setup UDP Socket (NEW)
// //     int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
// //     if (sock_fd < 0) {
// //         std::cerr << "Socket creation failed\n";
// //         return 1;
// //     }

// //     sockaddr_in server_addr{};
// //     server_addr.sin_family = AF_INET;
// //     server_addr.sin_addr.s_addr = INADDR_ANY; 
// //     server_addr.sin_port = htons(8080); // Listening on Port 8080

// //     if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
// //         std::cerr << "Bind failed. Port might be in use.\n";
// //         return 1;
// //     }

// //     std::cout << "[Writer] Listening for Live UDP Market Data on Port 8080...\n";

// //     Market incoming_data;

// //     // 3. The New Ingestion Loop
// //     while (true) {
// //         // This pauses the C++ program until a packet arrives
// //         ssize_t bytes_received = recvfrom(sock_fd, &incoming_data, sizeof(Market), 0, nullptr, nullptr);
        
// //         if (bytes_received == sizeof(Market)) {
// //             uint64_t head = layout->control.head.load(std::memory_order_relaxed);
            
// //             // Write directly to the Ring Buffer
// //             size_t idx = head & RING_BUFFER_MASK;
// //             layout->buffer[idx] = incoming_data;

// //             // Commit the write so Python sees it
// //             layout->control.head.store(head + 1, std::memory_order_release);
// //         }
// //         else {
// //             std::cout << "Dropped Packet: Got " << bytes_received 
// //                       << " bytes. Expected " << sizeof(Market) << " bytes.\n";
// //         }
// //     }
// // }


// #include "zcp_ipc.hpp"
// #include <iostream>
// #include <sys/mman.h>
// #include <fcntl.h>
// #include <unistd.h>

// int main() {
//     int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
//     ftruncate(fd, sizeof(SharedMemoryLayout));
//     auto* layout = static_cast<SharedMemoryLayout*>(
//         mmap(0, sizeof(SharedMemoryLayout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)
//     );

//     std::cout << "[Writer] Zero-Copy Memory Engine Online.\n";
//     std::cout << "[Writer] Generating data at MAXIMUM CPU speed...\n";

//     Market data;
//     data.symbol_id = 101;
//     data.price = 100.0;
//     data.volume = 10;

//     uint64_t local_head = 0;

//     while (true) {
//         data.price += 0.01;
//         data.volume++;

//         uint32_t slot = local_head % RING_BUFFER_SIZE;
//         layout->buffer[slot] = data;

//         local_head++;
//         layout->control.head.store(local_head, std::memory_order_release);
//     }

//     return 0;
// }


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

        uint32_t slot = local_head & RING_BUFFER_MASK; // Faster than modulo (%)
        layout->buffer[slot] = data;

        local_head++;
        // Memory barrier ensures data is physically written before head increments
        layout->control.head.store(local_head, std::memory_order_release);
    }

    return 0;
}