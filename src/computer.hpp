#pragma once

extern "C" {
    #include <sarch.h>
}
#include <stddef.h>

#define RANGE_CHECK(N, MIN, MAX) (N >= MIN && N < MAX)

#define ONLY_READ  0b01
#define ONLY_WRITE 0b10
#define READ_WRITE 0b11

enum {
    DEVICE_READ  = 1 << 0,
    DEVICE_WRITE = 1 << 1,
};

typedef unsigned char byte;

struct Device {
public:
    Device();
    Device(uint32_t from, uint32_t to, uint32_t priority, uint8_t readwrite_mask);
    ~Device();

    ReadFunc read;
    WriteFunc write;

    uint32_t from;
    uint32_t to;
    uint32_t priority;
    uint8_t readwrite_mask; // Bit 0 - can read, Bit 1 can write

    void* context;
};
