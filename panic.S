/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.extern	panic_dregs
	.extern	panic_aregs
	.extern	panic_handler
	.extern	_xcptsp

	.section .text

// sp+04: args... (moved to dregs/aregs)
// sp+00: pc
	.globl	panic
panic:
	// fake an exception with a
	// vector code of 0.
	move.w	%sr,-(%sp)
	move.w	#0,-(%sp)
	move.w	#0x2700,%sr
	move.w	#0x700,0xffff8240.w
	movem.l	8(%sp),%d0-%d7/%a0-%a6
	movem.l	%a0-%a7,panic_aregs
	movem.l	%d0-%d7,panic_dregs
	lea	_xcptsp,%sp
	lea	g_gd,%a6
	jmp	panic_handler

#include "elfattr.S"
