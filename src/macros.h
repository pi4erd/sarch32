#pragma once

#define READ8(CPU, BASE)        ((uint8_t) CPU->read(BASE))
#define READ16(CPU, BASE)       ((uint16_t)CPU->read(BASE) | (uint16_t)(CPU->read(BASE + 1) << 8))
#define READ32(CPU, BASE)       ((uint32_t)READ16(CPU, BASE) | ((uint32_t)READ16(CPU, BASE + 2)) << 16)
#define FORM32_16(A, B)         ((uint32_t)(A) | ((uint32_t)(B) << 16))
#define FORM16(A, B)            ((uint16_t)(A) | ((uint16_t)(B) << 8))
#define FORM32_8(A, B, C, D)    (FORM32_16(FORM_16(A, B), FORM_16(C, D)))
#define TODO()                  (fprintf(stderr, "NOT YET IMPLEMENTED"))