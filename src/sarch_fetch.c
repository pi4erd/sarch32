#include "sarch_fetch.h"
#include "macros.h"

#include <sarch.h>

#include <stdio.h>
#include <stdlib.h>


void fetch_opcode0(SArch32* sarch) {
    sarch->ar0 &= 0xFFFFFF00;
    sarch->ar0 |= READ8(sarch, sarch->ip++);
}

void fetch_opcode1(SArch32 *sarch)
{
    sarch->ar0 &= 0xFFFF00FF;
    sarch->ar0 |= (uint32_t)READ8(sarch, sarch->ip++) << 8;
}

void fetch_opcode(SArch32 *sarch)
{
    fetch_opcode0(sarch);

    if(sarch->ar0 & 0x80)
        fetch_opcode1(sarch);
}

#define INT_NUM_BITS 32

uint32_t rol(uint32_t n, uint8_t b) {
    if(b == 0 || b == INT_NUM_BITS) return n;
    return (n << b)|(n >> (INT_NUM_BITS - b));
}

void fetch_op_in_reg(SArch32* sarch, uint32_t* reg, uint8_t num_of_ops) {
    if(num_of_ops == 0)
        return;
    if(num_of_ops > 4) {
        fprintf(stderr, "(R) NUMBER OF OPERANDS ILLEGAL (%u)!\n", num_of_ops);
        exit(2);
    }
    for(uint8_t i = 0; i < num_of_ops; i++) {
        uint32_t mask = 0xFFFFFF00;
        mask &= rol(mask, i * 8);
        *reg |= (uint32_t)READ8(sarch, sarch->ip++) << (i * 8);
    }
}

void fetch_operand8(SArch32 *sarch, uint8_t number_of_operands)
{
    if(number_of_operands == 0)
        return;

    if(number_of_operands > 12) {
        fprintf(stderr, "NUMBER OF OPERANDS ILLEGAL (%u)!\n", number_of_operands);
        exit(2);
    }
    
    uint8_t r = number_of_operands / 4;
    uint8_t c = number_of_operands % 4;

    switch (r)
    {
    case 0:
        fetch_op_in_reg(sarch, &sarch->ar1, c);
        break;
    case 1:
        fetch_op_in_reg(sarch, &sarch->ar1, 4);
        fetch_op_in_reg(sarch, &sarch->ar2, c);
        break;
    case 2:
        fetch_op_in_reg(sarch, &sarch->ar1, 4);
        fetch_op_in_reg(sarch, &sarch->ar2, 4);
        fetch_op_in_reg(sarch, &sarch->ar3, c);
        break;
    case 3:
        fetch_op_in_reg(sarch, &sarch->ar1, 4);
        fetch_op_in_reg(sarch, &sarch->ar2, 4);
        fetch_op_in_reg(sarch, &sarch->ar3, 4);
        break;
    default:
        fprintf(stderr, "NUMBER OF REGISTERS ILLEGAL (%u)!\n", r);
        exit(2);
        break;
    }
}
