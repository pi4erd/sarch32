#include "../computer.hpp"

#include <cstdint>

#define IOINFO_SIZE 32

// This device, when queried, returns some information about devices on board
class IOInfo : public Device {
public:
    IOInfo(uint32_t addr, uint32_t priority, Span bios, Span ram, Span io, Span disk0);

    uint8_t read(uint32_t addr) override;

private:
    Span bios, ram, io, disk0;
};
