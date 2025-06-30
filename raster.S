/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "screen.h"
#include "mfp.h"
#include "vectors.h"
#include "asm_gd.h"

	.extern	schedule
	.extern	raster_irq_handler
	.extern	g_gd

	.section .text

#define WANT_COLORS	0

	.macro	eorcol val
#if WANT_COLORS
	eor.w	#\val,0xffff8240
#else
	eor.w	#0x000,0xffff8240
#endif
	.endm

	.macro	iraster val
	clr.b	MFP_TCRB
	move.b	#\val,MFP_TDRB
	move.b	#8,MFP_TCRB
	.endm

	.macro	sync
0:
	move.b	0xffff8209,%d0
	beq.s	0b
	not.b	%d0
	lsl.b	%d0,%d0
	.endm
/*
 * Raster (MFP Timer B) Interrupt @ IPL 6, MFP 8
 */
	.globl	raster
raster:
	eorcol	0x777
	iraster 198-SCREEN_HALF_Y	// penultimate line
	eorcol	0x777
	move.l	#preborder,V_TIMER_B	// next irq to pre-border
	addi.b	#1,g_gd+gd_irq_nest	// ++gd->irq_nest
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	lea	g_gd,%a6
	bclr.b	#0,gd_irq_thread_state(%a6)		// signal irq thread
	jsr	raster_irq_handler
	bclr.b	#0,MFP_IISA		// end-of-interrupt
	jmp	schedule

preborder:
	eorcol	0x777
	iraster 1			// last line
	move.l	#border,V_TIMER_B	// next irq to border
	move.b	MFP_IMRA,g_gd+gd_ras_mfp_imra
	move.b	MFP_IMRB,g_gd+gd_ras_mfp_imrb
	move.b	#0x01,MFP_IMRA		// disable non-raster MFP irqs
	move.b	#0x00,MFP_IMRB
	bclr.b	#0,MFP_IISA		// end-of-interrupt
	eorcol	0x777
	stop	#0x2500
	rte

#define	N_DBRA	19
#define	N_NOPS	8			// min 3, max 13, mid 8

border:
	eorcol	0x707			// 000->707 (magenta)
	clr.b	MFP_TCRB
	move.l	%d0,-(%sp)
	sync
	sync
	eorcol	0x777			// 707->070 (green)
	moveq	#N_DBRA,%d0
0:
	dbra	%d0,0b
	.rept	N_NOPS
	nop
	.endr
	eorcol	0x070			// 070->000 (black)
	move.b	#0,0xffff820a		// 60Hz
	.rept	8
	nop
	.endr
	move.b	#2,0xffff820a		// 50Hz
	move.b	g_gd+gd_ras_mfp_imra,MFP_IMRA	// restore masks
	move.b	g_gd+gd_ras_mfp_imrb,MFP_IMRB
	move.l	#raster,V_TIMER_B	// next irq to raster
	bclr.b	#0,MFP_IISA		// end-of-interrupt
	move.l	(%sp)+,%d0
	rte

/*
 * Falcon030 Raster (MFP Timer B) Interrupt @ IPL 6, MFP 8
 */
	.globl	raster_falcon
raster_falcon:
	addi.b	#1,g_gd+gd_irq_nest	// ++gd->irq_nest
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	lea	g_gd,%a6
	clr.b	MFP_TCRB
	bclr.b	#0,gd_irq_thread_state(%a6)		// signal irq thread
	jsr	raster_irq_handler
	bclr.b	#0,MFP_IISA		// end-of-interrupt
	jmp	schedule

#include "elfattr.S"
