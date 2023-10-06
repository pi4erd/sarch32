#include "computer.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <iterator>

std::vector<Device> devices;

#pragma region Declarations

#define K(N) N * 1024
#define M(N) N * 1024 * 1024
#define G(N) N * 1024 * 1024 * 1024

#define BIOS_SIZE M(1)
#define RAM_SIZE M(16)

void load_program_into(uint8_t* into, uint8_t* program, size_t program_size);
std::vector<uint8_t> load_program_from_file(const char* path);
void init_devices();
void destroy_devices();

void load_bios();

uint8_t read_devices(uint32_t addr);
void write_devices(uint32_t addr, uint8_t data);

#pragma endregion

int main() {
    init_devices();

    load_bios();

    SArch32* cpu = SArch32_new(read_devices, write_devices);
    cpu->log = true;

    while(!SArch32_is_halted(cpu)) {
        SArch32_step_clock(cpu);
    }

    printf("Stopped execution at %u steps!\n", cpu->total_cycles);

    destroy_devices();

    return 0;
}

#pragma region Definitions

void load_program_into(uint8_t *into, uint8_t *program, size_t program_size)
{
    std::memcpy(into, program, program_size);
}

std::vector<uint8_t> load_program_from_file(const char *path)
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

void init_devices()
{
    Device bios(0, BIOS_SIZE - 1, 0, DEVICE_READ);
    Device ram(0x1000000, 0x1000000 + RAM_SIZE - 1, 100, DEVICE_READ | DEVICE_WRITE);

    byte* bios_mem = (byte*)malloc(BIOS_SIZE);
    bios.context = bios_mem;

    auto bios_read = [](uint32_t addr) {
        return ((uint8_t*)devices[0].context)[addr];
    };
    bios.read = bios_read;

    devices.push_back(bios);

    byte* ram_mem = (byte*)malloc(RAM_SIZE);
    ram.context = ram_mem;

    auto ram_read = [](uint32_t addr) {
        return ((uint8_t*)devices[1].context)[addr];
    };
    auto ram_write = [](uint32_t addr, uint8_t data) {
        ((uint8_t*)devices[1].context)[addr] = data;
    };

    ram.read = ram_read;
    ram.write = ram_write;

    devices.push_back(ram);
}

void destroy_devices()
{
    for(auto &device : devices) {
        if(device.context != nullptr)
            free(device.context);
    }
}

void load_bios()
{
    std::vector<uint8_t> bios = load_program_from_file("bios.bin");

    load_program_into((uint8_t*)devices[0].context, bios.data(), bios.size());
}

uint8_t read_devices(uint32_t addr)
{
    Device *highestPriority = nullptr;

    for(auto &device : devices) {
        if(!(device.readwrite_mask & DEVICE_READ))
            continue;
        if(RANGE_CHECK(addr, device.from, device.to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device.priority)
                    highestPriority = &device;
            }
            else
                highestPriority = &device;
        }
    }

    if(highestPriority == nullptr)
        return (uint8_t)rand();
    return highestPriority->read(addr - highestPriority->from);
}

void write_devices(uint32_t addr, uint8_t data)
{
    Device *highestPriority = nullptr;

    for(auto &device : devices) {
        if(!(device.readwrite_mask & DEVICE_WRITE))
            continue;
        if(RANGE_CHECK(addr, device.from, device.to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device.priority)
                    highestPriority = &device;
            }
            else
                highestPriority = &device;
        }
    }

    if(highestPriority != nullptr)
        highestPriority->write(addr - highestPriority->from, data);
}

#pragma endregion
