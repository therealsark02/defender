/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .text

#include "asm_gd.h"

	.extern	_stack_top
	.extern	kill
	.extern exec

	.globl	disp
disp:
	lea	g_gd,%a1
	lea	gd_active(%a1),%a0
	bra.s	disp2
disp1:
	subq.b	#1,proc_ptime(%a0)
	bne.s	disp2
	move.l	%a0,gd_crproc(%a1)
	move.l	proc_paddr(%a0),%a1
	jmp	(%a1)
disp2:
	move.l	proc_plink(%a0),%a0
	cmp.w	#0,%a0
	bne.s	disp1
	move.l	_stack_top,%sp
	jmp	exec

	.globl	sleep
// sp -> [0:pc, 4:paddr, 8:ptime]
sleep:
	lea	g_gd,%a1
	move.l	gd_crproc(%a1),%a0
	move.b	11(%sp),proc_ptime(%a0)
	move.l	4(%sp),proc_paddr(%a0)
	bra.s	disp2

	.globl	sucide
sucide:
	lea	g_gd,%a1
	move.l	gd_crproc(%a1),-(%sp)
	jsr	kill
	addq	#4,%sp
	lea	g_gd,%a1
	move.l	%d0,%a0
	bra.s	disp2

#include "elfattr.S"
