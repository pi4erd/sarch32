#include "rom.hpp"

#include <fstream>
#include <cstdint>
#include <iterator>
#include <memory>
#include <cstring>
#include <iostream>
#include <exception>

Rom::Rom(std::string path, uint32_t start, uint32_t end, uint32_t priority, uint8_t rw_mask)
    : Device(start, end, priority, rw_mask)
{
    std::vector<uint8_t> program = load_program_from_file(path.c_str());

    size_t size = end - start;

    if(size < program.size()) {
        throw std::runtime_error("Program size smaller than ROM size");
    }

    memory = (uint8_t*)malloc(size);
    memcpy(memory, program.data(), program.size());
}

Rom::~Rom()
{
    free(memory);
}

std::vector<uint8_t> Rom::load_program_from_file(const char *path)
{
    using namespace std;

    vector<uint8_t> program;

    ifstream file(path, ios::binary);


    if(!file.is_open()) return program;

    file.unsetf(ios::skipws);

    streampos fileSize;
    
    file.seekg(ios::end);
    fileSize = file.tellg();
    file.seekg(ios::beg);

    program.reserve(fileSize);

    program.insert(program.begin(),
        istream_iterator<uint8_t>(file),
        istream_iterator<uint8_t>());
    
    file.close();

    return program;
}

uint8_t Rom::read(uint32_t addr)
{
    return memory[addr];
}