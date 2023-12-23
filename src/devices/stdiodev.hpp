#pragma once
#include "../computer.hpp"
#include "iodevice.hpp"

class Stdout : public IODevice {
public:
    Stdout(uint32_t addr, uint32_t priority, uint8_t rw_mask);

    uint8_t read(uint32_t addr) override;
    void write(uint32_t addr, uint8_t data) override;
};
