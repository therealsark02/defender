/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.extern	panic_dregs
	.extern	panic_aregs
	.extern	panic_handler

	.section .text

badxcpt:
	move.w	#0x2700,%sr
	move.w	#0x700,0xffff8240.w
	movem.l	%a0-%a7,panic_aregs
	movem.l	%d0-%d7,panic_dregs
	lea	_xcptsp,%sp
	lea	g_gd,%a6
	jmp	panic_handler

	.globl	xcpttab
xcpttab:
	.rept	78
	move.w	#4*(2+\+),-(%sp)
	bra	badxcpt
	.endr

	.section .bss

	.globl	_xcptsp
	.space	4096
_xcptsp:

