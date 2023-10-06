/**
 * @file sarch.h
 * @author @pi4erd
 * @brief Header file for SArch emulator
 * @version 0.1
 * @date 2023-09-08
 * 
 * @copyright Copyright (c) 2023
 * 
 * SArch32 - simple architecture 32 bit
 * Now that I use 32 bits, I can access up to 4GB of memory!
 * 
 * CPU always starts at address 0
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LIB_VERSION "0.2.0"

// MAX 32 FLAGS
enum STATUS_FLAGS {
    S_IL = 1 << 0, // Illegal
    S_HL = 1 << 1, // Halt
    S_ID = 1 << 2, // Interrupt Disabled
};

// MAX 32 FLAGS
enum MATH_FLAGS {
    M_OV = 1 << 0, // Overflow, signed math op overflows into sign bit
    M_CR = 1 << 1, // Carry
    M_NG = 1 << 2, // Negative
    M_ZR = 1 << 3, // Zero
};

/**
 * Interrupts are sent using special 1-byte code
 * Interrupt vector table has 256 addresses, or 1024 bytes of data
 * Interrupt vector space is addressed using 1-byte code
 * 
 * There are 2 special hardware interrupts for code 0xFE (IRQ) and 0xFF (NMI)
 * 
 * 0xFF (NMI) is the only non-maskable interrupt that interrupts no matter if
 * S_ID is set or not.
 * 
 * Interrupt table is located in last 1024 bytes of address space (0xFFFFFC00)
 * 
 */

enum INTERRUPTS {
    I_STACK_INTEGRITY_ERROR = 0xF0,
    MATH_OPERATION_ILLEGAL = 0xF1,
    IRQ = 0xFE,
    NMI = 0xFF,
};

typedef uint8_t (*ReadFunc)(uint32_t addr);
typedef void (*WriteFunc)(uint32_t addr, uint8_t data);
typedef uint8_t (*ReadPort)(uint8_t port);
typedef void (*WritePort)(uint8_t port, uint8_t data);

typedef struct _SARCH_BASE {
    /// DATA FOR EMULATION

    uint32_t total_cycles;
    uint32_t cycles; // Spent on instruction
    bool log;

    /// Inaccessible registers (needed for implementation)

    uint32_t ar0; // Used only for opcodes
    uint32_t ar1;
    uint32_t ar2;
    uint32_t ar3;

    /// Specific-purpose registers

    uint32_t ip; // Instruction pointer
    uint32_t sr; // Status register
    uint32_t mfr; // Math flags register
    uint32_t tptr; // Interrupt table pointer

    /**
     * Stack is a data structure with bytes in it.
     * The stack write in SArch32 works by, first, pushing to memory, then decreasing SP,
     * The stack read in SArch32 works by, first, increasing SP, then reading from memory.
     * 
     * SArch32 inherently supports stack integrity check. Before popping anything 
     * from the stack, checks if SP >= BP. If yes, sends interrupt number `0xF0`
     * which is I_STACK_INTEGRITY_ERROR.
     * 
     * Unfortunately, SArch32 doesn't check for stack overflow, which means that
     * the programmer will have to do checks themselves.
     * 
     * Data to stack is pushed in order: lo -> hi; and read in order: hi -> lo
     */

    uint32_t sp; // Stack pointer
    uint32_t bp; // Base pointer

    /// General-purpose registers

    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t r14;
    uint32_t r15;

    /// Functions

    ReadFunc read;
    WriteFunc write;
} SArch32;

typedef void (*InstrFunc)(struct _SARCH_BASE* context);

typedef struct _SARCH32INST {
    const char* name;
    const InstrFunc function;
    const uint32_t clock_cycles;
    const uint8_t fetch_cycles; // Used to determine amount of operands
    /**
     * @brief Fetch cycles:
     * 0 - no operand
     * 1 - 1 byte
     * 2 - 2 bytes
     * 3 - 3 bytes
     * and so on until
     * 12 - 12 bytes (all argument registers ar1, ar2, ar3)
     */
} Instruction;

#define SIZE_OF_INSTRUCTION(INSTRUCTION_OPCODE, FETCH_CYCLES) \
    ((INSTRUCTION_OPCODE & 0x80 ? 2 : 1) + FETCH_CYCLES)

SArch32* SArch32_new(ReadFunc read, WriteFunc write);
void SArch32_step_instruction(SArch32* sarch);
void SArch32_step_clock(SArch32* sarch);
void SArch32_reset(SArch32* sarch);
bool SArch32_is_halted(SArch32* sarch);
bool SArch32_illegal(SArch32* sarch);
void SArch32_destroy(SArch32* sarch);
