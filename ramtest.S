/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "asm_gd.h"
#include "screen.h"

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
	moveq	#(246-7)/2,ycnt
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

	// initialize the screen to all blue
	.globl	ramtest_scrinit
ramtest_scrinit:
	move.l	%d2,-(%sp)
	move.l	gd_vid_base(%a6),%a0
	addq	#8,%a0
	move.l	#SCREEN_HEIGHT-6,%d0
	move.l	#0xffff0000,%d1
	moveq	#0,%d2
0:
	move.l	%a0,%a1
	.rept	SCREEN_COLS-2
	move.l	%d1,(%a1)+
	move.l	%d2,(%a1)+
	.endr
	add.l	#160,%a0
	dbra	%d0,0b
	move.l	(%sp)+,%d2
	rts

