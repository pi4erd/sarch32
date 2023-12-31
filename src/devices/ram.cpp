#include "ram.hpp"

#include <cstdint>
#include <iostream>

Ram::Ram(uint32_t start, uint32_t end, uint32_t priority, uint8_t readwrite_mask)
    : Device(start, end, priority, readwrite_mask)
{
    size_t size = end - start;
    memory = (uint8_t*)malloc(size);
}

Ram::~Ram()
{
    free(memory);
}

uint8_t Ram::read(uint32_t addr)
{
    return memory[addr];
}

void Ram::write(uint32_t addr, uint8_t data)
{
    memory[addr] = data;
}
