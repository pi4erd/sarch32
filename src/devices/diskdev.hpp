#include "../computer.hpp"

#include <fstream>

#define BLOCK_SIZE 256

/**
 * @brief Disk device
 * 
 * Disk access:
 * 0 - 8: 64 bit choose block (256 bytes per block)
 * 8 - 9: 8 bit choose byte in block
 */
class Disk : public Device {
public:
    Disk(std::string path, uint32_t addr, uint32_t block_count, uint32_t priority, uint8_t rw_mask);
    ~Disk();

    void load_block(uint64_t ptr);
    void flush_block();

    uint8_t read(uint32_t addr);
    void write(uint32_t addr, uint8_t data);

private:
    uint64_t block_ptr;
    uint8_t byte_ptr;

    uint32_t block_count;
    bool busy = false;

    std::fstream disk_file;

    bool block_is_loaded;
    uint64_t loaded_block;
    uint8_t buffer[BLOCK_SIZE];
};