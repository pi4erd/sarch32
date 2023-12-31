#pragma once
#include "../computer.hpp"

#include <vector>
#include <string>

class Rom : public Device {
public:
    Rom(std::string path, uint32_t start, uint32_t end, uint32_t priority, uint8_t rw_mask);
    ~Rom();
    
    uint8_t read(uint32_t addr) override;

private:
    std::vector<uint8_t> load_program_from_file(const char *path);

    uint8_t* memory;
};
