#include "diskdev.hpp"

#include <exception>
#include <vector>

Disk::Disk(std::string path, uint32_t addr, uint32_t block_count, uint32_t priority, uint8_t rw_mask)
    : Device(addr, addr + 11, priority, rw_mask), block_count(block_count)
{
    disk_file.open(path, std::ios::binary | std::ios::out | std::ios::app);
    disk_file.close();
    disk_file.open(path, std::ios::binary | std::ios::out | std::ios::in);

    if(!disk_file.is_open()) {
        throw std::runtime_error("Failed to open file for reading!");
    }

    std::vector<char> empty(BLOCK_SIZE, 0);

    for(uint32_t i = 0; i < block_count; i++) {
        if(!disk_file.write(empty.data(), BLOCK_SIZE)) {
            throw std::runtime_error("Something went wrong writing to file!");
        }
    }

    block_is_loaded = false;
    loaded_block = 0;
}

Disk::~Disk()
{
    disk_file.close();
}

void Disk::load_block(uint64_t blk_ptr)
{
    if(blk_ptr >= block_count) {
        throw std::runtime_error("Block ptr outside of range! EH not implemented");
    }

    busy = true;

    if(block_is_loaded)
        flush_block();

    disk_file.seekg(blk_ptr * BLOCK_SIZE);
    std::string s;
    s.resize(BLOCK_SIZE);
    disk_file.read(&s[0], BLOCK_SIZE);

    busy = false;
    block_is_loaded = true;
    loaded_block = blk_ptr;
}

void Disk::flush_block()
{
    if(!block_is_loaded) return;

    if(loaded_block >= block_count) {
        throw std::runtime_error("Block ptr outside of range! EH not implemented");
    }

    busy = true;

    disk_file.seekp(loaded_block * BLOCK_SIZE);
    disk_file.write((char*)buffer, BLOCK_SIZE);
    disk_file.flush();

    busy = false;
}

uint8_t Disk::read(uint32_t addr)
{
    if(RANGE_CHECK(addr, 0, 8)) {
        // busy check
        return (uint8_t)busy;
    } else if(RANGE_CHECK(addr, 9, 10)) {
        if((block_is_loaded && (loaded_block != block_ptr)) || (!block_is_loaded))
            load_block(block_ptr);
        return buffer[byte_ptr];
    }
    return 0;
}

uint64_t rol(uint64_t n, uint8_t b) {
    if(b == 0 || b == 64) return n;
    return (n << b)|(n >> (64 - b));
}

void Disk::write(uint32_t addr, uint8_t data)
{
    if(RANGE_CHECK(addr, 0, 8)) {
        uint32_t mask_shift = addr * 8;
        uint64_t mask = rol(0xFF, mask_shift);
        block_ptr &= ~mask;
        block_ptr |= ((uint64_t)data) << mask_shift;
    } else if(RANGE_CHECK(addr, 8, 9)) {
        byte_ptr = data;
    } else if(RANGE_CHECK(addr, 9, 10)) {
        if((block_is_loaded && (loaded_block != block_ptr)) || (!block_is_loaded))
            load_block(block_ptr);
        buffer[byte_ptr] = data;
    } else if(RANGE_CHECK(addr, 10, 11)) {
        if(block_is_loaded)
            flush_block();
    }
}
