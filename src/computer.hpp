#pragma once

extern "C" {
    #include <sarch.h>
}
#include <cstdint>

#define RANGE_CHECK(N, MIN, MAX) (N >= MIN && N < MAX)

#define ONLY_READ  0b01
#define ONLY_WRITE 0b10
#define READ_WRITE 0b11

enum {
    DEVICE_READ  = 1 << 0,
    DEVICE_WRITE = 1 << 1,
};

typedef unsigned char byte;

struct Span {
public:
    Span() : from(0), to(0) {}
    Span(uint32_t from, uint32_t to) : from(from), to(to) {}

    uint32_t from;
    uint32_t to;
};

struct Device {
public:
    Device();
    Device(uint32_t from, uint32_t to, uint32_t priority, uint8_t readwrite_mask);
    ~Device();

    virtual uint8_t read(uint32_t addr) { return 0; };
    virtual void write(uint32_t addr, uint8_t data) {}

    Span span;
    uint32_t priority;
    uint8_t readwrite_mask; // Bit 0 - can read, Bit 1 can write
};
