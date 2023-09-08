#include <stdio.h>

#include <sarch.h>

uint8_t read(uint32_t addr) {

}

void write(uint32_t addr, uint8_t data) {

}

int main() {
    SArch32* cpu = SArch32_new(read, write);

    

    SArch32_destroy(cpu);

    return 0;
}