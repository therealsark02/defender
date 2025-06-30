/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "sysvars.h"

	.extern	_stack_top

#ifdef __ELF__
	.section	.text.startup
#else
	.text
#endif

	.globl	_start
_start:
	// go into super mode
	clr.l	-(%sp)
	move.w	#0x20,-(%sp)
	trap	#1
	addq	#6,%sp

	// grab the basepage and check
	// for the boot sector
	move.l	4(%sp),%a0
	move.l	_stack_top,%sp
	pea	0x80(%a0)
	pea	defender_str
	jsr	strcmp
	tst.l	%d0
	seq	%d7
	move.l	#g_gd,%a6
	move.l	%d7,-(%sp)
	jsr	sys_enter
0:
	bra.s   0b

	.section .data
defender_str:
	.string	"D E F E N D E R"

#include "elfattr.S"
