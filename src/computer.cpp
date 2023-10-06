#include "computer.hpp"

#include <cstdlib>

Device::Device()
{
}

Device::Device(uint32_t from, uint32_t to, uint32_t priority, uint8_t readwrite_mask)
    : from(from), to(to), priority(priority), readwrite_mask(readwrite_mask)
{
}

Device::~Device()
{
}
