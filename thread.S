/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "screen.h"
#include "vectors.h"
#include "mfp.h"
#include "asm_gd.h"

	.extern	vbi_irq_handler
	.extern	keyb_irq_handler
	.extern panic
	.extern	g_gd

	.section .text

	.macro	check_a6
	cmp.l	#g_gd,%a6
	beq.s	0f
	move.l	#0xed2ff434,-(%sp)
	jsr	panic
0:
	.endm

/*
 * VBL Interrupt @ IPL 4
 */
	.globl	hwvbi
hwvbi:
	addi.b	#1,g_gd+gd_irq_nest		// increment nesting level
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	lea	g_gd,%a6
	bclr.b	#0,gd_irq_thread_state(%a6)		// signal irq thread
	jsr	vbi_irq_handler
	bra	schedule

/*
 * Keyboard Interrupt @ IPL 6, MFP 6
 * To permit Timer B interrupts, reduce the 
 * processor IPL back to 5.
 */
	.globl	hwkeyb
hwkeyb:
	addi.b	#1,g_gd+gd_irq_nest		// increment nesting level
	move.w	#0x2500,%sr		// permit raster+timers
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	lea	g_gd,%a6
	jsr	key_irq_handler
	bclr.b	#6,MFP_IISB		// end-of-interrupt
	bra	schedule

/*
 * Restore the irq_thread enable/disable
 * If enabling, go to schedule to see
 * if we need to return to irq_thread.
 */
	.globl	irq_thread_restore
irq_thread_restore:
	check_a6
	move.w	%sr,-(%sp)		// save sr
	move.w	#0x2700,%sr		// disable interrupts
	bset.b	#2,gd_irq_thread_state(%a6)		// disable irq thread
	tst.b	9(%sp)			// flag non-zero?
	bne.s	1f			// yes, enable the irq thread
	rte
	.globl	irq_thread_restore_long
irq_thread_restore_long:
	check_a6
	move.l	(%sp)+,%d0		// pull return pc
	clr.w	-(%sp)			// format word
	move.l	%d0,-(%sp)		// push pc
	move.w	%sr,-(%sp)		// save sr
	move.w	#0x2700,%sr		// disable interrupts
	bset.b	#2,gd_irq_thread_state(%a6)		// disable irq thread
	tst.b	11(%sp)			// flag non-zero?
	bne.s	1f			// yes, enable the irq thread
	rte
1:
	bclr.b	#2,gd_irq_thread_state(%a6)		// enable irq thread
	movem.l	%d0-%d1/%a0-%a1/%a6,-(%sp)
	addi.b	#1,gd_irq_nest(%a6)		// increment irq nesting (for schedule)
	bra.s	schedule

	.globl	irq_thread_disable
irq_thread_disable:
	check_a6
	btst.b	#2,gd_irq_thread_state(%a6)
	seq.b	%d0
	bset.b	#2,gd_irq_thread_state(%a6)
	rts

// sp -> [d0-d1/a0-a1/a6,sr,pc]
	.globl	schedule
schedule:
	check_a6
	move.w	#0x2700,%sr		// lock-out while deciding
	sub.w	#1,gd_irq_thread_state(%a6)		// decrement nesting level
	beq.s	invoke_irq_thread
	// resume current thread (main or irq)
	movem.l	(%sp)+,%d0-%d1/%a0-%a1/%a6
	rte

	.globl	invoke_irq_thread
// sp -> [d0-d1/a0-a1/a6,sr,pc]
invoke_irq_thread:
	check_a6
	move.b	#3,gd_irq_thread_state(%a6)		// set enabled/active/~pending
	pea	irq_thread_return(%pc)
	move.l	#irq_thread,-(%sp)
	move.w	#0x2300,-(%sp)		// return with irqs enabled
	rte

	.globl	invoke_irq_thread_long
invoke_irq_thread_long:
	check_a6
	move.b	#3,gd_irq_thread_state(%a6)		// set enabled/active/~pending
	pea	irq_thread_return(%pc)
	clr.w	-(%sp)			// format word
	move.l	#irq_thread,-(%sp)
	move.w	#0x2300,-(%sp)		// return with irqs enabled
	rte

// sp -> [d0-d1/a0-a1/a6,sr,pc] (from main_thread)
irq_thread_return:
	check_a6
	move.w	#0x2700,%sr		// disable interrupts
	btst.b	#0,gd_irq_thread_state(%a6)		// irq_thread pending?
	beq.s	invoke_irq_thread	// yes, invoke irq thread
	bclr.b	#1,gd_irq_thread_state(%a6)		// clear irq_thread active
	// switch to main_thread
	movem.l	(%sp)+,%d0-%d1/%a0-%a1/%a6
	rte

