#pragma once
#include <sarch.h>

/**
 * Fetches opcode for an instruction
 * If instruction's 8th bit is set, fetches 2 bytes for opcode
 */
void fetch_opcode(SArch32 * sarch);
void fetch_operand8(SArch32 *sarch, uint8_t number_of_operands);
