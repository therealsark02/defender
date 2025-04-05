/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _VECTORS_H_
#define _VECTORS_H_

#define VECTOR_GENERATOR(mac) \
    mac(BUS_ERROR,      0x008) \
    mac(ADDRESS_ERROR,  0x00c) \
    mac(ILLEGAL_INSTR,  0x010) \
    mac(DIVIDE_BY_ZERO, 0x014) \
    mac(CHK_INSTR,      0x018) \
    mac(TRAPV_INSTR,    0x01c) \
    mac(PRIV_VIOLATION, 0x020) \
    mac(TRACE,          0x024) \
    mac(LINE_A,         0x028) \
    mac(LINE_F,         0x02c) \
    mac(SPURIOUS_IRQ,   0x060) \
    mac(IRQ_1,          0x064) \
    mac(IRQ_2,          0x068) \
    mac(IRQ_3,          0x06c) \
    mac(IRQ_4,          0x070) \
    mac(IRQ_5,          0x074) \
    mac(IRQ_6,          0x078) \
    mac(IRQ_7,          0x07c) \
    mac(TRAP_0,         0x080) \
    mac(TRAP_1,         0x084) \
    mac(TRAP_2,         0x088) \
    mac(TRAP_3,         0x08c) \
    mac(TRAP_4,         0x090) \
    mac(TRAP_5,         0x094) \
    mac(TRAP_6,         0x098) \
    mac(TRAP_7,         0x09c) \
    mac(TRAP_8,         0x0a0) \
    mac(TRAP_9,         0x0a4) \
    mac(TRAP_10,        0x0a8) \
    mac(TRAP_11,        0x0ac) \
    mac(TRAP_12,        0x0b0) \
    mac(TRAP_13,        0x0b4) \
    mac(TRAP_14,        0x0b8) \
    mac(TRAP_15,        0x0bc) \
    mac(MFP_IRQ_0,      0x100) \
    mac(MFP_IRQ_1,      0x104) \
    mac(MFP_IRQ_2,      0x108) \
    mac(MFP_IRQ_3,      0x10c) \
    mac(MFP_IRQ_4,      0x110) \
    mac(MFP_IRQ_5,      0x114) \
    mac(MFP_IRQ_6,      0x118) \
    mac(MFP_IRQ_7,      0x11c) \
    mac(MFP_IRQ_8,      0x120) \
    mac(MFP_IRQ_9,      0x124) \
    mac(MFP_IRQ_10,     0x128) \
    mac(MFP_IRQ_11,     0x12c) \
    mac(MFP_IRQ_12,     0x130) \
    mac(MFP_IRQ_13,     0x134) \
    mac(MFP_IRQ_14,     0x138) \
    mac(MFP_IRQ_15,     0x13c)

#define N_VECTORS       80      // 0 & 1 not used

#define VECTOR_ALIAS_GENERATOR(mac) \
    mac(VBI,          IRQ_4)        \
    mac(TIMER_B,      MFP_IRQ_8)    \
    mac(IKBD,         MFP_IRQ_6)    \
    mac(TIMER_D,      MFP_IRQ_4)

#ifdef __ASSEMBLER__

	.macro assign name value
	.equ \name,\value
	.endm

#define ASM_VECTOR_GENERATOR(a, b) \
    assign V_##a,b;
VECTOR_GENERATOR(ASM_VECTOR_GENERATOR)

#define ASM_VECTOR_ALIAS_GENERATOR(a, b) \
    assign V_##a,V_##b;
VECTOR_ALIAS_GENERATOR(ASM_VECTOR_ALIAS_GENERATOR)

#else

#define VECTOR_ENUM_GENERATOR(a, b) V_##a = b,
enum {
    VECTOR_GENERATOR(VECTOR_ENUM_GENERATOR)
};

#define VECTOR_ALIAS_ENUM_GENERATOR(a, b) V_##a = V_##b,
enum {
    VECTOR_ALIAS_GENERATOR(VECTOR_ALIAS_ENUM_GENERATOR)
};

void vectors_init(void);
void swapin_os_vectors(void);
void swapout_os_vectors(void);
extern const char xcpttab[126][8];

typedef void (*xcptfunc_t)(void);

static inline xcptfunc_t vector_get(uint32_t vec)
{
    return *(void **)vec;
}

static inline void vector_set(uint32_t vec, xcptfunc_t func)
{
    *(void **)vec = func;
}
#endif

#endif
