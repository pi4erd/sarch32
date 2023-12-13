#pragma region Includes

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
#include "macros.h"

#pragma endregion

#pragma region Decls and defs

void set_flag(SArch32* sarch, uint32_t flag, bool value);
bool get_flag(SArch32* sarch, uint32_t flag);
void set_mflag(SArch32* sarch, uint32_t flag, bool value);
bool get_mflag(SArch32* sarch, uint32_t flag);

void send_interrupt(SArch32* context, uint8_t code);
//void update_matflags(SArch32* context, uint32_t old); // TODO: Implement update matflags for ADD and SUB

#define HALT_ILLEGAL(CPU) (set_flag(CPU, S_HL, true), set_flag(CPU, S_IL, true))
#define ASSERT_EQUALS(A, B) (if(A != B) { fprintf(stderr, "ASSERT_EQUALS FAILED!"); exit(-1); })

#define FORM_REGISTER_LIST32(context) { \
        &context->r0, &context->r1, &context->r2, &context->r3, \
        &context->r4, &context->r5, &context->r6, &context->r7, \
        &context->r8, &context->r9, &context->r10, &context->r11, \
        &context->r12, &context->r13, &context->r14, &context->r15, \
        &context->ip, &context->sr, &context->mfr, &context->sp, \
        &context->bp, &context->tptr\
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

#define INTERRUPT_IRQ 0xFF
#define INTERRUPT_IMM 0xF

static uint32_t conditions[128] = {
    M_OV, M_CR, M_NG, M_ZR, M_GTR, M_LES, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    M_OV, M_CR, M_NG, M_ZR, M_GTR, M_LES, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    S_IL, S_HL, S_ID, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    S_IL, S_HL, S_ID, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
};

#pragma endregion

#pragma region Instruction Functions

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
        (reg0 == 16))
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (*(register_list[reg1]) & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] += *register_list[reg1];

    set_mflag(context, M_CR, (tmp > *register_list[reg0]) && 
                        (*register_list[reg1] > *register_list[reg0]));

    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

void i_add(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t num = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;

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

    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

// Load double word from memory
void loaddm(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t addr = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;
    
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

    uint32_t num = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;
    
    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = num;
}

void m_add(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t addr = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;
    
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
    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

void loadbm(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint32_t addr = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;
    
    if((reg0  > sizeof(register_list) / sizeof(void*))) 
    {
        HALT_ILLEGAL(context);
        return;
    }
    uint8_t tmp = READ8(context, addr);
    *register_list[reg0] = tmp;
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

// A conditional far-pointer jump
void jpc(SArch32* context) {
    // Should be about 128 valid conditions, maybe
    // First 64 conditions are: 32 math status positives, 32 math status negatives
    // Second 64 conditions are: 32 sr positives, 32 sr negatives
    // And last 128 mirrors first

    uint32_t addr = context->ar1;
    uint8_t condition = context->ar2 & 0xFF;

    bool neg = false;

    if(condition / 32 % 2 != 0) {
        neg = true;
    }

    if(condition / 64) {
        if(get_flag(context, conditions[condition]) != neg) context->ip = addr;
    } else {
        if(get_mflag(context, conditions[condition]) != neg) context->ip = addr;
    }
}

// An unconditional relative close-pointer jump
// A jump instruction with signed 32 bit offset value
// Relative address is calculated from the jump instruction opcode address in memory + offset
void jpr(SArch32* context) {
    uint32_t offset = context->ar1;

    context->ip += offset - SIZE_OF_INSTRUCTION(0, 4);
}

void jrc(SArch32* context) {
    uint32_t offset = context->ar1;
    uint8_t condition = context->ar2 & 0xFF;

    bool neg = false;

    if(condition / 32 % 2 != 0) {
        neg = true;
    }

    if(condition / 64) {
        if(get_flag(context, conditions[condition]) != neg)
            context->ip += offset - SIZE_OF_INSTRUCTION(0, 5);
    } else {
        if(get_mflag(context, conditions[condition]) != neg)
            context->ip += offset - SIZE_OF_INSTRUCTION(0, 5);
    }
}

// Pushes 32 bit register onto stack
void push(SArch32* context) {
    const uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    
    // Look at what this expands to and be glad I wrote macros XD
    PUSHSTACK32(context, *register_list[reg0]);
}

// Pops 32 bit register from stack
void pop(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);
    
    uint8_t reg0 = context->ar1 & 0xFF;

    *register_list[reg0] = POPSTACK32(context);
}

/**
 * SR -> STACK
 * IP -> STACK
 * AR1 -> IP
 */
void call(SArch32* context) {
    PUSHSTACK32(context, context->sr);
    PUSHSTACK32(context, context->ip);

    uint32_t addr = context->ar1;

    context->ip = addr;
}

/**
 * SR -> STACK
 * IP -> STACK
 * AR1 + (INSTRUCTION BASE) -> IP
 */
void callr(SArch32* context) {
    PUSHSTACK32(context, context->sr);
    PUSHSTACK32(context, context->ip);

    uint32_t offset = context->ar1;

    context->ip += offset - SIZE_OF_INSTRUCTION(0, 4);
}

/**
 * STACK -> IP
 * STACK -> SR
 */
void ret(SArch32* context) {
    uint32_t addr = POPSTACK32(context);
    uint32_t status = POPSTACK32(context);

    context->ip = addr;
    context->sr = status;
}

/**
 * R[X] <- R[Y]
 */
void movrd(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if ((reg0 > sizeof(register_list) / sizeof(void*)) || 
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16))
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = *register_list[reg1];
}

void movrw(SArch32* context) {
    uint16_t* register_list[] = FORM_REGISTER_LIST16(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if ((reg0 > sizeof(register_list) / sizeof(void*)) || 
        (reg1 > sizeof(register_list) / sizeof(void*)))
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = *register_list[reg1];
}

void movrb(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if ((reg0 > sizeof(register_list) / sizeof(void*)) || 
        (reg1 > sizeof(register_list) / sizeof(void*)))
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] = *register_list[reg1];
}

void intr(SArch32* context) {
    uint8_t code = context->ar1 & 0xFF;

    if(code == 0xFF || code == 0xFE)
    {
        HALT_ILLEGAL(context);
        return;
    }

    send_interrupt(context, code);
}

void i_sub(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t num = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (num & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] -= num;

    // TODO: Implement carry check for SUB
    //set_mflag(context, M_CR, (tmp > *register_list[reg0]) && (num > *register_list[reg0]));

    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

void m_sub(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t addr = context->ar1;

    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t num = READ32(context, addr);

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (num & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] -= num;

    // TODO: Implement carry check for SUB
    //set_mflag(context, M_CR, (tmp > *register_list[reg0]) && (num > *register_list[reg0]));

    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

void r_sub(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar2 & 0xFF;
    uint8_t reg1 = context->ar2 & 0xFF00;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    uint32_t carry = ((*(register_list[reg0]) & 0x80000000) & 
        (*register_list[reg1] & 0x80000000));

    uint32_t tmp = *register_list[reg0];

    *register_list[reg0] -= *register_list[reg1];

    // TODO: Implement carry check for SUB
    //set_mflag(context, M_CR, (tmp > *register_list[reg0]) && (num > *register_list[reg0]));

    set_mflag(context, M_NG, (*register_list[reg0] & 0x80000000) != 0);

    carry ^= (*(register_list[reg0]) & 0x80000000);

    set_mflag(context, M_OV, carry);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

// Negate integer register
void ngi(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) || (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    if(*register_list[reg0] == 0x80000000) {
        set_mflag(context, M_OV, 1);
        send_interrupt(context, MATH_OPERATION_ILLEGAL);
        return;
    }

    *register_list[reg0] = -*register_list[reg0];

    set_mflag(context, M_NG, *register_list[reg0] & 0x80000000);
    set_mflag(context, M_ZR, *register_list[reg0] == 0);
}

// Multiply signed double word registers
void r_mulsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = context->ar1 & 0xFF00;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] *= *register_list[reg1];

    // XXX: Do I use flags with multiply instructions?

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

// Divide signed double word registers
void r_divsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = context->ar1 & 0xFF00;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    if(*register_list[reg1] == 0) {
        send_interrupt(context, MATH_OPERATION_ILLEGAL);
        return;
    }

    *register_list[reg0] /= *register_list[reg1];

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

// Multiply unsigned double word registers
void r_mulud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = context->ar1 & 0xFF00;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] *= *register_list[reg1];

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

// Divide unsigned double word registers
void r_divud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;
    uint8_t reg1 = context->ar1 & 0xFF00;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1 > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    if(*register_list[reg1] == 0) {
        send_interrupt(context, MATH_OPERATION_ILLEGAL);
        return;
    }
    
    *register_list[reg0] /= *register_list[reg1];

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

void i_mulsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    int32_t num = context->ar1;
    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] *= num;

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

void i_divsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    int32_t num = context->ar1;
    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    if(num == 0) {
        send_interrupt(context, MATH_OPERATION_ILLEGAL);
        return;
    }

    *register_list[reg0] /= num;

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

void i_mulud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t num = context->ar1;
    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg0] *= num;

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
} 

void i_divud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t num = context->ar1;
    uint8_t reg0 = context->ar2 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    if(num == 0) {
        send_interrupt(context, MATH_OPERATION_ILLEGAL);
        return;
    }

    *register_list[reg0] /= num;

    set_mflag(context, M_OV, false);
    set_mflag(context, M_CR, false);
}

// Convert signed double word to float (int -> float)
void cvsdf(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    float value = (float)*register_list[reg0];

    *register_list[reg0] = CAST_TO_TYPE(value, int32_t);
}

// Convert float to signed double word
void cvfsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = context->ar1 & 0xFF;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16)) 
    {
        HALT_ILLEGAL(context);
        return;
    }

    float value = CAST_TO_FLOAT(*register_list[reg0]);

    *register_list[reg0] = (int32_t)value;
}

// imm compare signed double
void icmpsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    int32_t num = context->ar1;
    uint8_t reg = context->ar2 & 0xFF;

    if((reg  > sizeof(register_list) / sizeof(void*)) ||
        (reg == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg] == num);
    set_mflag(context, M_GTR, *register_list[reg] > num);
    set_mflag(context, M_LES, *register_list[reg] < num);
}

// imm compare unsigned double
void icmpud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint32_t num = context->ar1;
    uint8_t reg = context->ar2 & 0xFF;

    if((reg  > sizeof(register_list) / sizeof(void*)) ||
        (reg == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg] == num);
    set_mflag(context, M_GTR, *register_list[reg] > num);
    set_mflag(context, M_LES, *register_list[reg] < num);
}

// imm compare unsigned byte
void icmpub(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint8_t num = context->ar1 & 0xFF;
    uint8_t reg = (context->ar1 & 0xFF00) >> 8;

    if((reg  > sizeof(register_list) / sizeof(void*)))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg] == num);
    set_mflag(context, M_GTR, *register_list[reg] > num);
    set_mflag(context, M_LES, *register_list[reg] < num);
}

// imm compare unsigned word
void icmpuw(SArch32* context) {
    uint16_t* register_list[] = FORM_REGISTER_LIST16(context);

    uint16_t num = context->ar1 & 0xFFFF;
    uint8_t reg = (context->ar1 & 0xFF0000) >> 16;

    if((reg > sizeof(register_list) / sizeof(void*)))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg] == num);
    set_mflag(context, M_GTR, *register_list[reg] > num);
    set_mflag(context, M_LES, *register_list[reg] < num);
}

void rcmpsd(SArch32* context) {
    int32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16) || (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg0] == *register_list[reg1]);
    set_mflag(context, M_GTR, *register_list[reg0] > *register_list[reg1]);
    set_mflag(context, M_LES, *register_list[reg0] < *register_list[reg1]);
}

void rcmpud(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16) || (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg0] == *register_list[reg1]);
    set_mflag(context, M_GTR, *register_list[reg0] > *register_list[reg1]);
    set_mflag(context, M_LES, *register_list[reg0] < *register_list[reg1]);
}

void rcmpub(SArch32* context) {
    uint8_t* register_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg0] == *register_list[reg1]);
    set_mflag(context, M_GTR, *register_list[reg0] > *register_list[reg1]);
    set_mflag(context, M_LES, *register_list[reg0] < *register_list[reg1]);
}

void rcmpuw(SArch32* context) {
    uint16_t* register_list[] = FORM_REGISTER_LIST16(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    set_mflag(context, M_ZR, *register_list[reg0] == *register_list[reg1]);
    set_mflag(context, M_GTR, *register_list[reg0] > *register_list[reg1]);
    set_mflag(context, M_LES, *register_list[reg0] < *register_list[reg1]);
}

// Disable interrupts
void dsin(SArch32* context) {
    set_flag(context, S_ID, true);
}

// Enable interrupts
void esin(SArch32* context) {
    set_flag(context, S_ID, false);
}

// load from pointer at register into register
// M[R1] <- R0
void ldptrd(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16) || (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    WRITE32(context, *register_list[reg1], *register_list[reg0]);
}

void ldptrb(SArch32* context) {
    uint32_t* register32_list[] = FORM_REGISTER_LIST32(context);
    uint8_t* register8_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register8_list) / sizeof(void*)) ||
        (reg1  > sizeof(register32_list) / sizeof(void*)) ||
        (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    WRITE8(context, *register32_list[reg1], *register8_list[reg0]);
}

void ldptrw(SArch32* context) {
    uint32_t* register32_list[] = FORM_REGISTER_LIST32(context);
    uint16_t* register16_list[] = FORM_REGISTER_LIST16(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register16_list) / sizeof(void*)) ||
        (reg1  > sizeof(register32_list) / sizeof(void*)) ||
        (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    WRITE16(context, *register32_list[reg1], *register16_list[reg0]);
}

// Store to pointer at register from register
// M[R0] -> R1
void stptrd(SArch32* context) {
    uint32_t* register_list[] = FORM_REGISTER_LIST32(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register_list) / sizeof(void*)) ||
        (reg1  > sizeof(register_list) / sizeof(void*)) ||
        (reg0 == 16) || (reg1 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    *register_list[reg1] = READ32(context, *register_list[reg0]);
}

void stptrb(SArch32* context) {
    uint32_t* register32_list[] = FORM_REGISTER_LIST32(context);
    uint8_t* register8_list[] = FORM_REGISTER_LIST8(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register32_list) / sizeof(void*)) ||
        (reg1  > sizeof(register8_list) / sizeof(void*)) ||
        (reg0 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    *register8_list[reg1] = READ8(context, *register32_list[reg0]);
}

void stptrw(SArch32* context) {
    uint32_t* register32_list[] = FORM_REGISTER_LIST32(context);
    uint16_t* register16_list[] = FORM_REGISTER_LIST16(context);

    uint8_t reg0 = (context->ar1 & 0xFF);
    uint8_t reg1 = (context->ar1 & 0xFF00) >> 8;

    if((reg0  > sizeof(register32_list) / sizeof(void*)) ||
        (reg1  > sizeof(register16_list) / sizeof(void*)) ||
        (reg0 == 16))
    {
        //send_interrupt(context, ILLEGAL_OP); // TODO: Change HALT_ILLEGAL to interrupt
        HALT_ILLEGAL(context);
        return;
    }

    *register16_list[reg1] = READ16(context, *register32_list[reg0]);
}

void null_op(SArch32* context) {
    TODO();
}

void illegal(SArch32* context) {
    HALT_ILLEGAL(context);
}

const Instruction NULL_INSTR = {"ILLXINSTERRI", illegal, 0, 0};

 /// List of instructions. Instruction struct is as follows:
 /// - Name of the instruction (UPPERCASE)
 /// - Instruction function pointer with structure `void (*pointer)(SArch32*)`
 /// - Clock cycles needed to execute the instruction (basically any number)
 /// - Clock cycles needed to fetch the instruction's arguments (from 0 - 12 inclusive, HAS A FUNCTIONAL ROLE!)
 /// Instructions with opcode >=128 take at least 2 bytes
static const Instruction instructions[] = {
    // 0 0x0
    {"NOP", _nop, 2, 0}, {"HLT", halt, 3, 0}, {"RADD", r_add, 3, 2},
    {"IADD", i_add, 3, 5}, {"LOADM DW", loaddm, 2, 5}, {"LOADI DW", loaddi, 1, 5},

    // 6 0x6
    {"MADD", m_add, 4, 5}, {"LOADM B", loadbm, 1, 5}, {"LOADI B", loadbi, 1, 2},
    {"JMP", jmp, 2, 4}, {"JPC", jpc, 3, 5}, {"CALL", call, 4, 4},
    
    // 12 0xC
    {"JPR", jpr, 2, 4}, {"JRC", jrc, 3, 5}, {"CALLR", callr, 4, 4},
    {"PUSH", push, 4, 1}, {"POP", pop, 4, 1}, {"RET", ret, 4, 0},

    // 18 0x12
    {"MOVR DW", movrd, 1, 2}, {"MOVR W", movrw, 1, 2}, {"MOVR B", movrb, 1, 2},
    {"INT", intr, 4, 1}, {"ISUB", i_sub, 3, 5}, {"MSUB", m_sub, 4, 5},
    
    // 24 0x18
    {"RSUB", r_sub, 3, 2}, {"NGI", ngi, 2, 1}, {"RMULSD", r_mulsd, 6, 2},
    {"RDIVSD", r_divsd, 7, 2}, {"RMULUD", r_mulud, 6, 2}, {"RDIVUD", r_divud, 7, 2},

    // 30 0x1E
    {"IMULSD", i_mulsd, 6, 5}, {"IDIVSD", i_divsd, 7, 5}, {"IMULUD", i_mulud, 6, 5},
    {"IDIVUD", i_divud, 7, 5}, {"CVSDF", cvsdf, 5, 1}, {"CVFSD", cvfsd, 7, 1},

    // 36 0x24 // TODO: Implement comparison instructions
    {"ICMPSD", icmpsd, 3, 5}, {"ICMPUD", icmpud, 3, 5}, {"ICMPUB", icmpub, 2, 2},
    {"ICMPUW", icmpuw, 2, 3}, {"RCMPSD", rcmpsd, 2, 2}, {"RCMPUD", rcmpud, 2, 2},

    // 42 0x2A
    {"RCMPUB", rcmpub, 2, 2}, {"RCMPUW", rcmpuw, 2, 2}, {"DSIN", dsin, 1, 0},
    {"ESIN", esin, 1, 0}, {"LDPTRD", ldptrd, 4, 2}, {"LDPTRB", ldptrb, 2, 2},

    // 48 0x30
    {"LDPTRW", ldptrw, 3, 2}, {"STPTRD", stptrd, 4, 2}, {"STPTRB", stptrb, 2, 2},
    {"STPTRW", stptrw, 2, 2}
};

#pragma endregion

#pragma region CPU Stuff

SArch32 *SArch32_new(ReadFunc read, WriteFunc write)
{
    SArch32* result = (SArch32*)malloc(sizeof(SArch32));

    result->read = read;
    result->write = write;

    SArch32_reset(result);

    result->log = 0;

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
    uint16_t opc = (sarch->ar0 & 0x0000FFFF);

    if(!(opc & 0x80))
        opc &= 0xFF;

    if(opc >= sizeof(instructions) / sizeof(Instruction)) {
        HALT_ILLEGAL(sarch);
        return;
    }

    fetch_operand8(sarch, instructions[opc].fetch_cycles);

    instructions[opc].function(sarch);

    if(sarch->log)
        printf("%s\n", instructions[opc].name);
}

void SArch32_step_clock(SArch32 *sarch)
{
    if(get_flag(sarch, S_IL))
        return;
    
    // get opcode w/o modifying IP
    fetch_opcode(sarch);
    uint16_t opc = sarch->ar0 & 0xFFFF;

    sarch->ip--;
    if(opc & 0x80) {
        sarch->ip--;
    }

    if(opc >= sizeof(instructions) / sizeof(Instruction)) {
        HALT_ILLEGAL(sarch);
        return;
    }

    sarch->cycles++;
    sarch->total_cycles++;

    if(sarch->cycles >= instructions[opc].clock_cycles + instructions[opc].fetch_cycles) {
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

void SArch32_reset(SArch32 *sarch)
{
    sarch->ar0 = 0;
    sarch->ar1 = 0;
    sarch->ar2 = 0;
    sarch->ar3 = 0;

    sarch->sp = 0;
    sarch->bp = 0;

    sarch->sr = 0;
    sarch->mfr = 0;

    sarch->ip = 0;

    sarch->total_cycles = 0;
    sarch->cycles = 0;
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

bool get_flag(SArch32 *sarch, uint32_t flag)
{
    if(sarch->sr & flag)
        return 1;
    else return 0;
}

void set_mflag(SArch32 *sarch, uint32_t flag, bool value)
{
    if(value) {
        sarch->mfr |= flag;
    } else {
        sarch->mfr &= ~flag;
    }
}

bool get_mflag(SArch32 *sarch, uint32_t flag)
{
    if(sarch->mfr & flag)
        return 1;
    else return 0;
}

void send_interrupt(SArch32 *context, uint8_t code)
{
    // FIXME: Maybe find a way to make interrupts better? I don't know yet. Testing will show
    uint32_t addr = (uint32_t)code * 4 + context->tptr;

    if(code != NMI && !get_flag(context, S_ID)) {
        return;
    }

    PUSHSTACK32(context, context->sr);
    PUSHSTACK32(context, context->ip);

    context->ip = addr;
}

#pragma endregion
