#pragma once
#include "../computer.hpp"

class IODevice : public Device {
public:
    IODevice(uint32_t addr, uint32_t priority, uint8_t rw_mask);
};