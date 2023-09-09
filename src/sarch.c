#include <sarch.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #error "NOT YET IMPLEMENTED, DON'T USE WIN32" // TODO
#else
    #include <unistd.h>
#endif

void set_flag(SArch32* sarch, uint32_t flag, bool value);
uint32_t get_flag(SArch32* sarch, uint32_t flag);
void set_mflag(SArch32* sarch, uint32_t flag, bool value);
uint32_t get_mflag(SArch32* sarch, uint32_t flag);

#define HALT_ILLEGAL(CPU) (set_flag(CPU, S_HL, true), set_flag(CPU, S_IL, true))

#define READ16(CPU, ADDR) ((uint16_t)CPU->read(ADDR) | (uint16_t)(CPU->read(ADDR + 1) << 8))

void _nop(SArch32* context) {
    asm inline("nop");
}

void halt(SArch32* context) {
    set_flag(context, S_HL, true);
}

static const Instruction instructions[] = {
    {"NOP", _nop, 2}, {"HLT", halt, 3}
};

void fetch_opcode(SArch32* sarch) {
    sarch->ar0 = ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

void fetch_operand0(SArch32* sarch) {
    sarch->ar1 = ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

void fetch_operand1(SArch32* sarch) {
    sarch->ar2 = ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

void fetch_operand2(SArch32* sarch) {
    sarch->ar3 = ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

SArch32 *SArch32_new(ReadFunc read, WriteFunc write)
{
    SArch32* result = (SArch32*)malloc(sizeof(SArch32));

    result->read = read;
    result->write = write;

    return result;
}

void SArch32_step_instruction(SArch32 *sarch)
{
    if(get_flag(sarch, S_IL))
        return;

    fetch_opcode(sarch);
    uint16_t opc = (sarch->ar0 & 0xFFFF0000) >> 16;

    if(opc >= sizeof(instructions) / sizeof(Instruction)) {
        HALT_ILLEGAL(sarch);
        return;
    }

    instructions[opc].function(sarch);

    if(sarch->log)
        printf("%s\n", instructions[opc].name);
}

void SArch32_step_clock(SArch32 *sarch)
{
    if(get_flag(sarch, S_IL))
        return;
    
    // get opcode w/o modifying IR
    uint16_t opc = (READ16(sarch, sarch->ip) & 0xFFFF0000) >> 16;

    if(opc >= sizeof(instructions) / sizeof(Instruction)) {
        HALT_ILLEGAL(sarch);
        return;
    }

    sarch->cycles++;
    sarch->total_cycles++;

    if(sarch->cycles > instructions[opc].clock_cycles) {
        SArch32_step_instruction(sarch);
        sarch->cycles = 0;
    }
    const uint32_t nanos = 100000; // Clock speed = 10 kHz
    struct timespec remaining, request = { 0, nanos };

    int result = nanosleep(&request, &remaining);
    if(result) {
        fprintf(stderr, "Error occured while trying to nanosleep!\n");
        exit(1);
    }
}

bool SArch32_is_halted(SArch32 *sarch)
{
    return get_flag(sarch, S_HL);
}

void SArch32_destroy(SArch32 *sarch)
{
    free(sarch);
}

void set_flag(SArch32 *sarch, uint32_t flag, bool value)
{
    if(value) {
        sarch->sr |= flag;
    } else {
        sarch->sr &= ~flag;
    }
}

uint32_t get_flag(SArch32 *sarch, uint32_t flag)
{
    return sarch->sr & flag;
}

void set_mflag(SArch32 *sarch, uint32_t flag, bool value)
{
    if(value) {
        sarch->mfr |= flag;
    } else {
        sarch->mfr &= ~flag;
    }
}

uint32_t get_mflag(SArch32 *sarch, uint32_t flag)
{
    return sarch->mfr & flag;
}
