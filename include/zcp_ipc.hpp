#pragma once
#include <atomic>
#include <cstdint>
#include <cstddef>

constexpr size_t RING_BUFFER_SIZE = 1024;
constexpr size_t RING_BUFFER_MASK = RING_BUFFER_SIZE - 1;

constexpr const char* SHM_NAME = "/zcp_shm";

// Data Structure
struct Market {
    uint32_t symbol_id;
    double price;
    uint32_t volume;
};

// The control structure
struct SharedControl {
    alignas(64) std::atomic<uint64_t> head{0};
    
    char padding[64 - sizeof(std::atomic<uint64_t>)];
    
    alignas(64) std::atomic<uint64_t> tail{0};
};

// Layout in RAM
struct SharedMemoryLayout {
    SharedControl control;
    Market buffer[RING_BUFFER_SIZE];
};