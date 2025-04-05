/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "screen.h"
#include "vectors.h"
#include "timers.h"
#include "mfp.h"
#include "asm_gd.h"
#include "ym2149.h"

	.extern panic
	.extern	g_gd

	.section .text

	.macro	sfx_entry
	addq.b	#1,g_gd+gd_irq_nest		// increment nesting level
	move.w	#0x2500,%sr
	move.l	%a0,-(%sp)
	move.l	t_sptr,%a0
	.endm

	.macro	sfx_eof_check
	cmp.l	t_eptr,%a0
	blo.s	1f
	move.l	t_rptr,%a0
	cmp.w	#0,%a0
	bne.s	1f
	and.b	#MFP_TCCD_MASK,MFP_TCCD
1:
	move.l	%a0,t_sptr
	.endm

	.macro	sfx_exit
	bclr.b	#MFP_PSGTIMER_BIT,MFP_IISB
	move.l	(%sp)+,%a0
	cmp.w	#1,g_gd+gd_irq_thread_state
	beq.s	1f
	sub.b	#1,g_gd+gd_irq_nest		// decrement nesting level
	rte
1:
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	lea	g_gd,%a6
	jmp	schedule
	.endm

# 1-channel, not packed
	.globl	sfxplay1
sfxplay1:
	sfx_entry
	move.b	#YM_AVOL,YM_SELECT
	move.b	(%a0)+,YM_DATA
	sfx_eof_check
	sfx_exit

# 1-channel, packed, even-phase
	.globl	sfxplay1p
sfxplay1p:
	sfx_entry
	move.l	%d0,-(%sp)
	move.b	(%a0),%d0
	lsr.b	#4,%d0
	move.b	#YM_AVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.l	(%sp)+,%d0
	lea	sfxplay1pb(%pc),%a0
	move.l	%a0,V_PSG_TIMER
	sfx_exit

# 1-channel, packed, odd-phase
sfxplay1pb:
	sfx_entry
	move.l	%d0,-(%sp)
	move.b	(%a0)+,%d0
	andi.b	#0xf,%d0
	move.b	#YM_AVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.l	(%sp)+,%d0
	sfx_eof_check
	lea	sfxplay1p(%pc),%a0
	move.l	%a0,V_PSG_TIMER
	sfx_exit

# 2-channel, not packed - not supported
	.globl	sfxplay2
sfxplay2:
	move.l	#0x12ed5534,-(%sp)
	jsr	panic

# 2-channel, packed
# (avol:bvol),(avol:bvol),...
	.globl	sfxplay2p
sfxplay2p:
	sfx_entry
	move.l	%d0,-(%sp)
	move.b	(%a0),%d0
	lsr.b	#4,%d0
	move.b	#YM_AVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.b	(%a0)+,%d0
	andi.b	#0xf,%d0
	move.b	#YM_BVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.l	(%sp)+,%d0
	sfx_eof_check
	sfx_exit

# 3-channel, not packed - not supported
	.globl	sfxplay3
sfxplay3:
	move.l	#0x12edd334,-(%sp)
	jsr	panic

# 3-channel, packed, even-phase
# (avol:bvol),(cvol:avol),(bvol:cvol)
	.globl	sfxplay3p
sfxplay3p:
	sfx_entry
	move.l	%d0,-(%sp)
	move.b	(%a0),%d0
	lsr.b	#4,%d0
	move.b	#YM_AVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.b	(%a0)+,%d0
	andi.b	#0xf,%d0
	move.b	#YM_BVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.b	(%a0),%d0
	lsr.b	#4,%d0
	move.b	#YM_CVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.l	(%sp)+,%d0
	sfx_eof_check
	lea	sfxplay3pb(%pc),%a0
	move.l	%a0,V_PSG_TIMER
	sfx_exit

# 3-channel, packed, odd-phase
# (cvol:avol),(bvol:cvol),(avol:bvol)...
sfxplay3pb:
	sfx_entry
	move.l	%d0,-(%sp)
	move.b	(%a0)+,%d0
	andi.b	#0xf,%d0
	move.b	#YM_AVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.b	(%a0),%d0
	lsr.b	#4,%d0
	move.b	#YM_BVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.b	(%a0)+,%d0
	andi.b	#0xf,%d0
	move.b	#YM_CVOL,YM_SELECT
	move.b	%d0,YM_DATA
	move.l	(%sp)+,%d0
	sfx_eof_check
	lea	sfxplay3p(%pc),%a0
	move.l	%a0,V_PSG_TIMER
	sfx_exit

