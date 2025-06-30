/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "asm_gd.h"

	.section .text

	.globl	ramtest_step
#define colptr0	%a0
#define colptr1	%a1
#define scrptr	%a2
#define mask	%d0
#define A	%d1
#define B	%d2
#define l0	%d3
#define l1	%d4
#define ycnt	%d5

ramtest_step:
	movem.l	%d2-%d7/%a2-%a5,-(%sp)
	move.l	gd_crproc(%a6),%a5
	move.l	proc_ramtest_colptr0(%a5),colptr0
	move.l	proc_ramtest_colptr1(%a5),colptr1
	move.l	proc_ramtest_ptr(%a5),scrptr
	move.l	proc_ramtest_mask(%a5),mask
	move.b	proc_ramtest_a(%a5),A
	move.b	proc_ramtest_b(%a5),B
	moveq	#(240-7)/2,ycnt
loop:
	move.b	B,%d7
	not.b	%d7
	and.b	#9,%d7
	bne.s	1f
0:
	lsr.b	#1,A
	roxr.b	#1,B
	or.b	#0x80,A
	bra.s	2f
1:
	move.b	B,%d7
	and.b	#9,%d7
	beq.s	0b
	lsr.b	#1,A
	roxr.b	#1,B
2:
	// A color
	move.b	A,%d7
	lsr.b	#1,%d7
	and.w	#0x78,%d7
	move.l	0(colptr0,%d7.w),l0
	move.l	4(colptr0,%d7.w),l1
	move.b	A,%d7
	lsl.b	#3,%d7
	and.w	#0x78,%d7
	or.l	0(colptr1,%d7.w),l0
	or.l	4(colptr1,%d7.w),l1
	move.l	(scrptr),%d7
	and.l	mask,%d7
	or.l	l0,%d7
	move.l	%d7,(scrptr)
	move.l	4(scrptr),%d7
	and.l	mask,%d7
	or.l	l1,%d7
	move.l	%d7,4(scrptr)
	lea	160(scrptr),scrptr
	
	// B color
	move.b	B,%d7
	lsr.b	#1,%d7
	and.w	#0x78,%d7
	move.l	0(colptr0,%d7.w),l0
	move.l	4(colptr0,%d7.w),l1
	move.b	B,%d7
	lsl.b	#3,%d7
	and.w	#0x78,%d7
	or.l	0(colptr1,%d7.w),l0
	or.l	4(colptr1,%d7.w),l1
	move.l	(scrptr),%d7
	and.l	mask,%d7
	or.l	l0,%d7
	move.l	%d7,(scrptr)
	move.l	4(scrptr),%d7
	and.l	mask,%d7
	or.l	l1,%d7
	move.l	%d7,4(scrptr)
	lea	160(scrptr),scrptr
	
	dbra	ycnt,loop

	move.b	A,proc_ramtest_a(%a5)
	move.b	B,proc_ramtest_b(%a5)

	movem.l	(%sp)+,%d2-%d7/%a2-%a5
	rts

#include "elfattr.S"
