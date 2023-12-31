#pragma once
#include "../computer.hpp"

class Ram : public Device {
public:
    Ram(uint32_t start, uint32_t end, uint32_t priority, uint8_t readwrite_mask);
    ~Ram();
    
    uint8_t read(uint32_t addr) override;
    void write(uint32_t addr, uint8_t data) override;

private:
    uint8_t *memory;
};
