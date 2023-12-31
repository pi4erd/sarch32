#include "stdiodev.hpp"

#include <iostream>

Stdout::Stdout(uint32_t addr, uint32_t priority, uint8_t rw_mask)
    : IODevice(addr, priority, rw_mask)
{
}

uint8_t Stdout::read(uint32_t addr)
{
    return 0;
}

void Stdout::write(uint32_t addr, uint8_t data)
{
    std::cout << data;
}
