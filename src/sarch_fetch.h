#pragma once
#include <sarch.h>

#define READ16(CPU, BASE) ((uint16_t)CPU->read(BASE) | (uint16_t)(CPU->read(BASE + 1) << 8))
#define READ32(CPU, BASE) ((uint32_t)READ16(CPU, BASE) | ((uint32_t)READ16(CPU, BASE + 2)) << 16)
#define FORM32(A, B)      ((A) | ((B) << 16))
#define FORM16(A, B)      ((A) | ((B) << 8))
#define TODO()            (fprintf(stderr, "NOT YET IMPLEMENTED"))

void fetch_opcode(SArch32 * sarch);
void fetch_operand16_0(SArch32 *sarch);
void fetch_operand16_1(SArch32 *sarch);
void fetch_operand16_2(SArch32 *sarch);
void fetch_operand16_3(SArch32 *sarch);
