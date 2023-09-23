#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#ifdef _WIN32
    #error "NOT YET IMPLEMENTED, DON'T USE WIN32" // TODO
#else
    #include <unistd.h>
#endif

#include <sarch.h>

/**
 * @brief A range check macro (min inclusive, max exclusive). True if in range
 * 
 */
#define RANGE_CHECK(N, MIN, MAX) (N >= MIN && N < MAX)

typedef struct {
    uint8_t* ptr;
    size_t size;
} ByteArray;

static SArch32* cpu = NULL;
static ByteArray ram;

uint8_t _read(uint32_t addr) {
    if(RANGE_CHECK(addr, 0, ram.size))
        return ram.ptr[addr];
    return rand();
}

void _write(uint32_t addr, uint8_t data) {
    if(addr >= ram.size)
        return;
    ram.ptr[addr] = data;
}

void run_realistic() {
    if(!cpu) return;

    while(!SArch32_is_halted(cpu)) {
        SArch32_step_clock(cpu);
    }
}

void run_simulated() {
    if(!cpu) return;

    struct timespec remaining, request = { 0, 10000000 };

    while(!SArch32_is_halted(cpu)) {
        SArch32_step_instruction(cpu);
        int r = nanosleep(&request, &remaining);
        if(r) {
            fprintf(stderr, "Error occured: nanosleep didn't work!\n");
            exit(1);
        }
    }
}

int main() {
    srand(time(NULL));
    cpu = SArch32_new(_read, _write);

    cpu->log = true;

    ram.size = 1024 * 1024;
    ram.ptr = malloc(ram.size); // Allocate 1MiB

    const uint8_t program[] = {
        0x08, 0x00, 0x00, 0x01, // loadbi
        0x08, 0x00, 0x01, 0x02, // loadbi
        0x08, 0x00, 0x02, 0x03, // loadbi
        0x08, 0x00, 0x03, 0x04, // loadbi
        0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, // loadbm 4 <- 0
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00 // jmp 0
    };

    memcpy(ram.ptr, program, sizeof(program));

    run_realistic();

    if(SArch32_illegal(cpu)) {
        printf("ILLEGAL INSTRUCTION HALT OCCURED!\n");
        return 1;
    }

    printf("0x%08x 0x%08x\n", cpu->r0, cpu->r1);

    SArch32_destroy(cpu);

    free(ram.ptr);

    return 0;
}