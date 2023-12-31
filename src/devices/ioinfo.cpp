#include "ioinfo.hpp"

IOInfo::IOInfo(uint32_t addr, uint32_t priority, Span bios, Span ram, Span io, Span disk0)
    : Device(addr, addr + IOINFO_SIZE, priority, ONLY_READ), ram(ram), bios(bios), io(io), disk0(disk0)
{
}

uint8_t IOInfo::read(uint32_t addr)
{
    uint32_t spans[IOINFO_SIZE / sizeof(uint32_t)] = {
        bios.from, bios.to,
        ram.from, ram.to,
        io.from, io.to,
        disk0.from, disk0.to
    };
    if(RANGE_CHECK(addr, 0, IOINFO_SIZE))
        return *(((uint8_t*)spans) + addr);
    return 0;
}
