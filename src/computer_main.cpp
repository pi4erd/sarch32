#define N_DEBUG

#include "computer.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <iterator>

#include "devices/iodevice.hpp"
#include "devices/ram.hpp"
#include "devices/rom.hpp"
#include "devices/stdiodev.hpp"
#include "devices/diskdev.hpp"

std::vector<Device*> devices;

#pragma region Defines

#define K(N) N * 1024
#define M(N) N * 1024 * 1024
#define G(N) N * 1024 * 1024 * 1024

#define BIOS_START 0
#define BIOS_SIZE M(1)
#define BIOS_END BIOS_START + BIOS_SIZE - 1

#define RAM_START BIOS_END + 1
#define RAM_SIZE M(16)
#define RAM_END RAM_START + RAM_SIZE - 1

#define STDIO_START RAM_END + 1
#define STDIO_END STDIO_START + 1

#define DISK0_IO_START STDIO_END + 1

#define CLOCK_SPEED_KHZ 10.0

#pragma endregion


#pragma region Declarations

void init_devices();
void destroy_devices();

uint8_t read_devices(uint32_t addr);
void write_devices(uint32_t addr, uint8_t data);

#pragma endregion


int main() {
    init_devices();

    SArch32* cpu = SArch32_new(read_devices, write_devices);
    cpu->log = false;

    while(!SArch32_is_halted(cpu)) {
#ifndef _DEBUG
        SArch32_step_clock(cpu, CLOCK_SPEED_KHZ);
#else
        SArch32_step_instruction(cpu);
        getc(stdin);
#endif
    }

    if(SArch32_illegal(cpu)) {
        printf("\nIllegal instruction reached!\n");
    }

    printf("Stopped execution at %u steps!\n", cpu->total_cycles);

    SArch32_destroy(cpu);

    destroy_devices();

    return 0;
}


#pragma region Definitions

void init_devices()
{
    Rom* bios = new Rom("bios.bin", BIOS_START, BIOS_END, 0, ONLY_READ);
    Ram* ram = new Ram(RAM_START, RAM_END, 100, READ_WRITE);
    Stdout* io = new Stdout(STDIO_START, 100, READ_WRITE);

    // create disk with 20 blocks of 256 ?= 5120 bytes
    Disk* disk0 = new Disk("disk.raw", DISK0_IO_START, 20, 100, READ_WRITE);

    devices.push_back(bios);
    devices.push_back(ram);
    devices.push_back(io);
    devices.push_back(disk0);
}

void destroy_devices()
{
    for(auto &device : devices) {
        if(device->context != nullptr)
            free(device->context);
        delete device;
    }
}

uint8_t read_devices(uint32_t addr)
{
    Device *highestPriority = nullptr;

    for(auto &device : devices) {
        if(!(device->readwrite_mask & DEVICE_READ))
            continue;
        if(RANGE_CHECK(addr, device->from, device->to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device->priority)
                    highestPriority = device;
            }
            else
                highestPriority = device;
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
        if(!(device->readwrite_mask & DEVICE_WRITE))
            continue;
        if(RANGE_CHECK(addr, device->from, device->to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device->priority)
                    highestPriority = device;
            }
            else
                highestPriority = device;
        }
    }

    if(highestPriority != nullptr)
        highestPriority->write(addr - highestPriority->from, data);
}

#pragma endregion
