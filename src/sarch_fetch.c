#include "sarch_fetch.h"
#include <sarch.h>

void fetch_opcode(SArch32* sarch) {
    sarch->ar0 = ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

void fetch_operand16_0(SArch32* sarch) {
    sarch->ar1 |= ((uint32_t)READ16(sarch, sarch->ip));
    sarch->ip += 2;
}

void fetch_operand16_1(SArch32* sarch) {
    sarch->ar1 |= ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}

void fetch_operand16_2(SArch32* sarch) {
    sarch->ar2 |= ((uint32_t)READ16(sarch, sarch->ip));
    sarch->ip += 2;
}

void fetch_operand16_3(SArch32* sarch) {
    sarch->ar2 |= ((uint32_t)READ16(sarch, sarch->ip)) << 16;
    sarch->ip += 2;
}
