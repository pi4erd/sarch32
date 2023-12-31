#define N_DEBUG

#include "computer.hpp"

#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>

#include "devices/iodevice.hpp"
#include "devices/ram.hpp"
#include "devices/rom.hpp"
#include "devices/stdiodev.hpp"
#include "devices/diskdev.hpp"
#include "devices/ioinfo.hpp"

std::vector<std::unique_ptr<Device>> devices;

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

uint8_t read_devices(uint32_t addr);
void write_devices(uint32_t addr, uint8_t data);

#pragma endregion


int main() {
    init_devices();

    SArch32* cpu = SArch32_new(read_devices, write_devices);
#ifndef _DEBUG
    cpu->log = false;
#else
    cpu->log = true;
#endif

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

    return 0;
}


#pragma region Definitions

void init_devices()
{
    std::unique_ptr<Rom> bios = std::make_unique<Rom>("bios.bin", BIOS_START, BIOS_END, 0, ONLY_READ);
    std::unique_ptr<Ram> ram = std::make_unique<Ram>(RAM_START, RAM_END, 100, READ_WRITE);
    std::unique_ptr<Stdout> io = std::make_unique<Stdout>(STDIO_START, 100, READ_WRITE);

    // create disk with 20 blocks of 256 ?= 5120 bytes
    std::unique_ptr<Disk> disk0 = std::make_unique<Disk>("disk.raw", DISK0_IO_START, 20, 100, READ_WRITE);

    std::unique_ptr<IOInfo> ioinfo = std::make_unique<IOInfo>(0x100000000 - IOINFO_SIZE, 
        100, bios->span, ram->span, io->span, disk0->span);

    devices.push_back(std::move(bios));
    devices.push_back(std::move(ram));
    devices.push_back(std::move(io));
    devices.push_back(std::move(disk0));
    devices.push_back(std::move(ioinfo));
}

uint8_t read_devices(uint32_t addr)
{
    Device *highestPriority = nullptr;

    for(auto &device : devices) {
        if(!(device->readwrite_mask & DEVICE_READ))
            continue;
        if(RANGE_CHECK(addr, device->span.from, device->span.to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device->priority)
                    highestPriority = device.get();
            }
            else
                highestPriority = device.get();
        }
    }

    if(highestPriority == nullptr)
        return (uint8_t)rand();
    return highestPriority->read(addr - highestPriority->span.from);
}

void write_devices(uint32_t addr, uint8_t data)
{
    Device *highestPriority = nullptr;

    for(auto &device : devices) {
        if(!(device->readwrite_mask & DEVICE_WRITE))
            continue;
        if(RANGE_CHECK(addr, device->span.from, device->span.to)) {
            if(highestPriority != nullptr) {
                if(highestPriority->priority < device->priority)
                    highestPriority = device.get();
            }
            else
                highestPriority = device.get();
        }
    }

    if(highestPriority != nullptr)
        highestPriority->write(addr - highestPriority->span.from, data);
}

#pragma endregion
