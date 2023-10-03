#pragma once

#define WRITE8(CPU, ADDR, DATA)     (CPU->write(ADDR, DATA))
#define WRITE16(CPU, ADDR, DATA)    (WRITE8(CPU, ADDR, (DATA) & 0xFF), \
                            WRITE8(CPU, ADDR + 1, ((DATA) & 0xFF00) >> 8))
#define WRITE32(CPU, ADDR, DATA)    (WRITE16(CPU, ADDR, (DATA) & 0xFFFF), \
                            WRITE16(CPU, ADDR + 2, ((DATA) & 0xFFFF0000) >> 16))

#define READ8(CPU, BASE)            ((uint8_t) CPU->read(BASE))
#define READ16(CPU, BASE)           ((uint16_t)CPU->read(BASE) | (uint16_t)(CPU->read(BASE + 1) << 8))
#define READ32(CPU, BASE)           ((uint32_t)READ16(CPU, BASE) | ((uint32_t)READ16(CPU, BASE + 2)) << 16)

#define POPSTACK8(CPU)              (READ8(CPU, ++CPU->sp))
#define POPSTACK16(CPU)             (((uint16_t)POPSTACK8(CPU) << 8) | (uint16_t)POPSTACK8(CPU))
#define POPSTACK32(CPU)             (((uint32_t)POPSTACK16(CPU) << 16) | (uint32_t)POPSTACK16(CPU))
#define PUSHSTACK8(CPU, DATA)       (WRITE8(CPU, CPU->sp--, DATA))
#define PUSHSTACK16(CPU, DATA)      (PUSHSTACK8(CPU, (DATA) & 0xFF), \
                            PUSHSTACK8(CPU, ((DATA) & 0xFF00) >> 8))
#define PUSHSTACK32(CPU, DATA)      (PUSHSTACK16(CPU, (DATA) & 0xFFFF), \
                            PUSHSTACK16(CPU, ((DATA) & 0xFFFF0000) >> 16))

#define FORM32_16(A, B)             ((uint32_t)(A) | ((uint32_t)(B) << 16))
#define FORM16(A, B)                ((uint16_t)(A) | ((uint16_t)(B) << 8))
#define FORM32_8(A, B, C, D)        (FORM32_16(FORM_16(A, B), FORM_16(C, D)))
#define TODO()                      (fprintf(stderr, "NOT YET IMPLEMENTED\n"), exit(2))
