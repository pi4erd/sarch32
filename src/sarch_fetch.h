#pragma once
#include <sarch.h>

/**
 * Fetches opcode for an instruction
 * If instruction's 8th bit is set, fetches 2 bytes for opcode
 */
void fetch_opcode(SArch32 *sarch);
void fetch_operand8(SArch32 *sarch, uint8_t number_of_operands);

uint8_t popstack8(SArch32 *sarch);
uint16_t popstack16(SArch32 *sarch);
uint32_t popstack32(SArch32 *sarch);

void pushstack8(SArch32 *sarch, uint8_t data);
void pushstack16(SArch32 *sarch, uint16_t data);
void pushstack32(SArch32 *sarch, uint32_t data);
