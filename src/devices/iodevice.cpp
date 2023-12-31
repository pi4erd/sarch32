#include "iodevice.hpp"

IODevice::IODevice(uint32_t addr, uint32_t priority, uint8_t rw_mask)
    : Device(addr, addr + 1, priority, rw_mask)
{
}
