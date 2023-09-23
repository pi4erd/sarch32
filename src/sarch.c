#include <sarch.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
    #error "NOT YET IMPLEMENTED, DON'T USE WIN32" // TODO
#else
    #include <unistd.h>
#endif

#include "sarch_fetch.h"

void set_flag(SArch32* sarch, uint32_t flag, bool value);
uint32_t get_flag(SArch32* sarch, uint32_t flag);
void set_mflag(SArch32* sarch, uint32_t flag, bool value);
uint32_t get_mflag(SArch32* sarch, uint32_t flag);

#define HALT_ILLEGAL(CPU) (set_flag(CPU, S_HL, true), set_flag(CPU, S_IL, true))
#define ASSERT_EQUALS(A, B) (if(A != B) { fprintf(stderr, "ASSERT_EQUALS FAILED!"); exit(-1); })

#pragma region Instruction Functions

#define FORM_REGISTER_LIST32(context) { \
        &context->r0, &context->r1, &context->r2, &context->r3, \
        &context->r4, &context->r5, &context->r6, &context->r7, \
        &context->r8, &context->r9, &context->r10, &context->r11, \
        &context->r12, &context->r13, &context->r14, &context->r15, \
        &context->ip, &context->sr, &context->mfr, &context->sp, \
        &context->bp, \
    }

#define FORM_REGISTER_LIST16(context) { \
        (uint16_t*)&context->r0, (uint16_t*)&context->r0 + 1, \
        (uint16_t*)&context->r1, (uint16_t*)&context->r1 + 1, \
        (uint16_t*)&context->r2, (uint16_t*)&context->r2 + 1, \
        (uint16_t*)&context->r3, (uint16_t*)&context->r3 + 1, \
        (uint16_t*)&context->r4, (uint16_t*)&context->r4 + 1, \
        (uint16_t*)&context->r5, (uint16_t*)&context->r5 + 1, \
        (uint16_t*)&context->r6, (uint16_t*)&context->r6 + 1, \
        (uint16_t*)&context->r7, (uint16_t*)&context->r7 + 1, \
        (uint16_t*)&context->r8, (uint16_t*)&context->r8 + 1, \
        (uint16_t*)&context->r9, (uint16_t*)&context->r9 + 1, \
        (uint16_t*)&context->r10, (uint16_t*)&context->r10 + 1, \
        (uint16_t*)&context->r11, (uint16_t*)&context->r11 + 1, \
        (uint16_t*)&context->r12, (uint16_t*)&context->r12 + 1, \
        (uint16_t*)&context->r13, (uint16_t*)&context->r13 + 1, \
        (uint16_t*)&context->r14, (uint16_t*)&context->r14 + 1, \
        (uint16_t*)&context->r15, (uint16_t*)&context->r15 + 1, \
    }

#define FORM_REGISTER_LIST8(context) { \
        (uint8_t*)&context->r0,     (uint8_t*)&context->r0 + 1, \
        (uint8_t*)&context->r0 + 2, (uint8_t*)&context->r0 + 3, \
        (uint8_t*)&context->r1,     (uint8_t*)&context->r1 + 1, \
        (uint8_t*)&context->r1 + 2, (uint8_t*)&context->r1 + 3, \
        (uint8_t*)&context->r2,     (uint8_t*)&context->r2 + 1, \
        (uint8_t*)&context->r2 + 2, (uint8_t*)&context->r2 + 3, \
        (uint8_t*)&context->r3,     (uint8_t*)&context->r3 + 1, \
        (uint8_t*)&context->r3 + 2, (uint8_t*)&context->r3 + 3, \
        (uint8_t*)&context->r4,     (uint8_t*)&context->r4 + 1, \
        (uint8_t*)&context->r4 + 2, (uint8_t*)&context->r4 + 3, \
        (uint8_t*)&context->r5,     (uint8_t*)&context->r5 + 1, \
        (uint8_t*)&context->r5 + 2, (uint8_t*)&context->r5 + 3, \
        (uint8_t*)&context->r6,     (uint8_t*)&context->r6 + 1, \
        (uint8_t*)&context->r6 + 2, (uint8_t*)&context->r6 + 3, \
        (uint8_t*)&context->r7,     (uint8_t*)&context->r7 + 1, \
        (uint8_t*)&context->r7 + 2, (uint8_t*)&context->r7 + 3, \
    }

void _nop(SArch32* context) {
    asm inline("nop");
}

void halt(SArch32* context) {
    set_flag(context, S_HL, true);
}

void r_add(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0 > sizeof(register_list) / sizeof(void*)) || 
        (reg1 > sizeof(register_list) / sizeof(void*)) || 
        (reg0 == 16) || (reg1 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (*(register_list[reg1]) & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] += *register_list[reg1];

    // TODO: NEEDS TESTING
    set_mflag(context, M_CR, (tmp > *register_list[reg0]) && 
                        (*register_list[reg1] > *register_list[reg0]));

    set_mflag(context, M_NG, *(register_list[reg0]) & 0x80000000 != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
}

void i_add(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;

    uint32_t num = FORM32((context->ar1 & 0xFFFF0000) >> 16, 
                            context->ar2 & 0x0000FFFF);

    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (num & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] += num;

    set_mflag(context, M_CR, (tmp > *register_list[reg0]) && (num > *register_list[reg0]));

    set_mflag(context, M_NG, *(register_list[reg0]) & 0x80000000 != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
}

// Load double word from memory
void loaddm(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0x00FF;

    uint32_t addr = FORM32((context->ar1 & 0xFFFF0000) >> 16, 
                            context->ar2 & 0x0000FFFF);
    
    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = READ32(context, addr);
}

// Load double word immediate
void loaddi(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0x00FF;

    uint32_t num = FORM32((context->ar1 & 0xFFFF0000) >> 16, 
                            context->ar2 & 0x0000FFFF);
    
    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = num;
}

void m_add(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0x00FF;

    uint32_t addr = FORM32((context->ar1 & 0xFFFF0000) >> 16, 
                            context->ar2 & 0x0000FFFF);
    
    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t num = READ32(context, addr);

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (num & 0x80000000));
    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] += num;

    set_mflag(context, M_CR, (tmp > *register_list[reg0]) && (num > *register_list[reg0]));
    set_mflag(context, M_NG, *(register_list[reg0]) & 0x80000000 != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
}

void loadbm(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = context->ar1 & 0x00FF;

    uint32_t addr = FORM32((context->ar1 & 0xFFFF0000) >> 16, 
                            context->ar2 & 0x0000FFFF);
    
    if((reg0  > sizeof(register_list) / sizeof(void*))) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = context->read(addr);
}

void loadbi(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = context->ar1 & 0x00FF;

    uint8_t num = (context->ar1 & 0x0000FF00) >> 8;
    
    if((reg0  > sizeof(register_list) / sizeof(void*))) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = num;
}

// An unconditional far-pointer jump
void jmp(SArch32* context) {
    uint32_t addr = context->ar1;

    context->ip = addr;
}

void null_op(SArch32*) { TODO(); }

static const Instruction instructions[] = {
    {"NOP", _nop, 2, 0}, {"HLT", halt, 3, 0}, {"RADD", r_add, 3, 1},
    {"IADD", i_add, 3, 3}, {"LOADM DW", loaddm, 2, 3}, {"LOADI DW", loaddi, 1, 3},
    {"MADD", m_add, 4, 3}, {"LOADM B", loadbm, 1, 3}, {"LOADI B", loadbi, 1, 1},
    {"JMP", jmp, 2, 2}
};

#pragma endregion

#pragma region CPU Stuff

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

    sarch->ar0 = 0;
    sarch->ar1 = 0;
    sarch->ar2 = 0;
    sarch->ar3 = 0;

    fetch_opcode(sarch);
    uint16_t opc = (sarch->ar0 & 0xFFFF0000) >> 16;

    if(opc >= sizeof(instructions) / sizeof(Instruction)) {
        HALT_ILLEGAL(sarch);
        return;
    }

    switch (instructions[opc].fetch_cycles)
    {
    case 0:
        break;
    case 1:
        fetch_operand16_0(sarch);
        break;
    case 2:
        fetch_operand16_0(sarch);
        fetch_operand16_1(sarch);
        break;
    case 3:
        fetch_operand16_0(sarch);
        fetch_operand16_1(sarch);
        fetch_operand16_2(sarch);        
        break;
    case 4:
        fetch_operand16_0(sarch);
        fetch_operand16_1(sarch);
        fetch_operand16_2(sarch);        
        fetch_operand16_3(sarch);
        break;
    default:
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

    if(sarch->cycles > instructions[opc].clock_cycles + instructions[opc].fetch_cycles) {
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

bool SArch32_illegal(SArch32 *sarch)
{
    return get_flag(sarch, S_IL);
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

#pragma endregion