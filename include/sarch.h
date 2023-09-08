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
 */
#pragma once

#include <stdint.h>

typedef uint8_t (*ReadFunc)(uint32_t);
typedef void (*WriteFunc)(uint32_t, uint8_t);
typedef uint8_t (*ReadPort)(uint8_t);
typedef void (*WritePort)(uint8_t, uint8_t data);

typedef struct _SARCH_BASE {
    /// Inaccessible registers (needed for implementation)

    uint32_t ar0;
    uint32_t ar1;
    uint32_t ar2;
    uint32_t ar3;

    /// Specific-purpose registers

    uint32_t ip;
    uint32_t sp; // stack pointer
    uint32_t bp; // base pointer

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

    ReadFunc read;
    WriteFunc write;
    ReadPort read_port;
    WritePort write_port;
} SArch32;

SArch32* SArch32_new(ReadFunc read, WriteFunc write);
void SArch32_destroy(SArch32* sarch);
