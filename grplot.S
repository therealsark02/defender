/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .text

	.globl plot_ground
plot_ground:
	movea.l 4(%sp),%a1
	movea.l 8(%sp),%a0
	movem.l	 %a2-%a3,-(%sp)
	move.w	#17,%d0
grnxt:
	movea.l %a1,%a2			//  4:	a2 = dst
	adda.w	(%a0)+,%a2		// 12:	a2 = dst + src->offs
	movea.l (%a0)+,%a3		// 12:	a3 = src->func
	jmp	(%a3)			//  8:	call

gpl16:	move.w	(%a0)+,( 0 * 160)(%a2)
gpl15:	move.w	(%a0)+,( 1 * 160)(%a2)
gpl14:	move.w	(%a0)+,( 2 * 160)(%a2)
gpl13:	move.w	(%a0)+,( 3 * 160)(%a2)
gpl12:	move.w	(%a0)+,( 4 * 160)(%a2)
gpl11:	move.w	(%a0)+,( 5 * 160)(%a2)
gpl10:	move.w	(%a0)+,( 6 * 160)(%a2)
gpl9:	move.w	(%a0)+,( 7 * 160)(%a2)
gpl8:	move.w	(%a0)+,( 8 * 160)(%a2)
gpl7:	move.w	(%a0)+,( 9 * 160)(%a2)
gpl6:	move.w	(%a0)+,(10 * 160)(%a2)
gpl5:	move.w	(%a0)+,(11 * 160)(%a2)
gpl4:	move.w	(%a0)+,(12 * 160)(%a2)
gpl3:	move.w	(%a0)+,(13 * 160)(%a2)
gpl2:	move.w	(%a0)+,(14 * 160)(%a2)
gpl1:	move.w	(%a0)+,(15 * 160)(%a2)
	addq	#8,%a1				// 4:  dst += 8
	dbra	%d0,grnxt			// 10:  loop
	movem.l	 (%sp)+,%a2-%a3
	rts

	.globl ground_ftab
ground_ftab:
	.long	gpl1
	.long	gpl2
	.long	gpl3
	.long	gpl4
	.long	gpl5
	.long	gpl6
	.long	gpl7
	.long	gpl8
	.long	gpl9
	.long	gpl10
	.long	gpl11
	.long	gpl12
	.long	gpl13
	.long	gpl14
	.long	gpl15
	.long	gpl16

	.globl erase_ground
erase_ground:
	movea.l 4(%sp),%a1
	movea.l 8(%sp),%a0
	movem.l	 %a2-%a3,-(%sp)
	move.w	#17,%d0
grenxt:
	movea.l %a1,%a2			//  4:	a2 = dst
	adda.w	(%a0)+,%a2		// 12:	a2 = dst + src->offs
	movea.l (%a0)+,%a3		// 12:	a3 = src->func
	jmp	(%a3)			//  8:	call

ger16:	clr.w	( 0 * 160)(%a2)
ger15:	clr.w	( 1 * 160)(%a2)
ger14:	clr.w	( 2 * 160)(%a2)
ger13:	clr.w	( 3 * 160)(%a2)
ger12:	clr.w	( 4 * 160)(%a2)
ger11:	clr.w	( 5 * 160)(%a2)
ger10:	clr.w	( 6 * 160)(%a2)
ger9:	clr.w	( 7 * 160)(%a2)
ger8:	clr.w	( 8 * 160)(%a2)
ger7:	clr.w	( 9 * 160)(%a2)
ger6:	clr.w	(10 * 160)(%a2)
ger5:	clr.w	(11 * 160)(%a2)
ger4:	clr.w	(12 * 160)(%a2)
ger3:	clr.w	(13 * 160)(%a2)
ger2:	clr.w	(14 * 160)(%a2)
ger1:	clr.w	(15 * 160)(%a2)
	addq	#8,%a1				//  4:  dst += 8
	dbra	%d0,grenxt			// 10:  loop
	movem.l	 (%sp)+,%a2-%a3
	rts

	.globl ground_eftab
ground_eftab:
	.long	ger1
	.long	ger2
	.long	ger3
	.long	ger4
	.long	ger5
	.long	ger6
	.long	ger7
	.long	ger8
	.long	ger9
	.long	ger10
	.long	ger11
	.long	ger12
	.long	ger13
	.long	ger14
	.long	ger15
	.long	ger16

