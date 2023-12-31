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
#define PUSHSTACK8(CPU, DATA)       (WRITE8(CPU, CPU->sp--, DATA))

#define FORM32_16(A, B)             ((uint32_t)(A) | ((uint32_t)(B) << 16))
#define FORM16(A, B)                ((uint16_t)(A) | ((uint16_t)(B) << 8))
#define FORM32_8(A, B, C, D)        (FORM32_16(FORM_16(A, B), FORM_16(C, D)))
#define TODO()                      (fprintf(stderr, "NOT YET IMPLEMENTED\n"), exit(2))

#define CAST_TO_FLOAT(U)            *(float*)&U
#define CAST_TO_UINT(F)             *(uint32_t*)&F
#define CAST_TO_TYPE(V, T)          *(T*)&V
