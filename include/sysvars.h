/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SYSVARS_H_
#define _SYSVARS_H_

#define SYSVAR_GENERATOR(mac) \
	mac(uint32_t, 0x400, etv_timer) \
	mac(uint32_t, 0x42e, phystop) \
	mac(uint32_t, 0x432, _membot) \
	mac(uint32_t, 0x436, _memtop) \
	mac(uint16_t, 0x442, _timr_ms) \
	mac(uint16_t, 0x452, vblsem) \
	mac(uint16_t, 0x454, nvbls) \
	mac(uint32_t, 0x456, _vblqueue) \
	mac(uint32_t, 0x462, _vbclock) \
	mac(uint32_t, 0x466, _frclock) \
	mac(uint32_t, 0x4ba, _hz_200) \
	mac(uint32_t, 0x4ce, _vbl_list) \
	mac(uint16_t, 0x59e, _longframe) \
	mac(uint32_t, 0x5a0, _p_cookies)

#ifdef __ASSEMBLER__

	.macro sysvar_assign name value
    .globl \name
	.equ \name,\value
	.endm

#define ASM_SYSVAR_GEN(a, b, c) \
    sysvar_assign c, b;

SYSVAR_GENERATOR(ASM_SYSVAR_GEN)

#else

#define SYSVAR_DECL_GEN(a, b, c) extern a c;
SYSVAR_GENERATOR(SYSVAR_DECL_GEN)

#endif

#endif

