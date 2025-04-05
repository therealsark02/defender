/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .text

//   +8 -> data   [m00 d00 d01 m01 d10 d11]
// sp+4 -> screen [s00 s01 s10 s11]
// sp+0 -> ra
	.macro def_plot fname adj reg nrows func
	.globl \fname
\fname:
	move.l	4(%sp), %a0
	move.l	8(%sp), %a1
	.ifnb \adj
	lea	\adj(%a1),%a1
	.endif
	.ifnb \reg
	movem.l	%d2-\reg, -(%sp)
	.endif
	.rep \nrows
	\func
	.endr
	.ifnb \reg
	movem.l	(%sp)+, %d2-\reg
	.endif
	rts
	.endm

	.macro plot_full
	movem.l	(%a0),%d0-%d3	// 44: fetch 32x4-bit pixels (16 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1, pixels 0..15
	and.l	(%a1)+,%d1	// 14: mask planes 2&3, pixels 0..15
	or.l	(%a1)+,%d0	// 14: merge data planes 0&1, pix 0..15
	or.l	(%a1)+,%d1	// 14: merge data planes 2&3, pix 0..15
	and.l	(%a1),%d2	// 14: mask planes 0&1, pixels 16..31
	and.l	(%a1)+,%d3	// 14: mask planes 2&3, pixels 16..31
	or.l	(%a1)+,%d2	// 14: merge data planes 0&1, pix 16..31
	or.l	(%a1)+,%d3	// 14: merge data planes 2&3, pix 16..31
	movem.l %d0-%d3,(%a0)	// 40: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	.macro plot_left
	movem.l	(%a0),%d0-%d1	// 28: fetch 16x4-bit pixels (8 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1
	and.l	(%a1)+,%d1	// 14: mask planes 2&3
	or.l	(%a1)+,%d0	// 14: merge data planes 0&1
	or.l	(%a1)+,%d1	// 14: merge data planes 2&3
	lea	12(%a1), %a1	//  8: skip unused mask+data
	movem.l %d0-%d1,(%a0)	// 24: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	.macro plot_right
	movem.l	(%a0), %d0-%d1	// 28: fetch 16x4-bit pixels (8 bytes)
	lea	12(%a1), %a1	//  8: skip unused mask+data
	and.l	(%a1),%d0	// 14: mask planes 0&1
	and.l	(%a1)+,%d1	// 14: mask planes 2&3
	or.l	(%a1)+, %d0	// 14: merge data planes 0&1
	or.l	(%a1)+, %d1	// 14  merge data planes 2&3
	movem.l	%d0-%d1, (%a0)	// 24: store right 16 pixels
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	def_plot plot_1x1_full   ""   "%d3" 1 plot_full
	def_plot plot_1x1_left   ""   ""    1 plot_left
	def_plot plot_1x1_right  ""   ""    1 plot_right

	def_plot plot_1x3_full   ""   "%d3" 3 plot_full
	def_plot plot_1x3_left   ""   ""    3 plot_left
	def_plot plot_1x3_right  ""   ""    3 plot_right

	def_plot plot_1x4_full   ""   "%d3" 4 plot_full
	def_plot plot_1x4_left   ""   ""    4 plot_left
	def_plot plot_1x4_right  ""   ""    4 plot_right

	def_plot plot_1x6_full   ""   "%d3" 6 plot_full
	def_plot plot_1x6_left   ""   ""    6 plot_left
	def_plot plot_1x6_right  ""   ""    6 plot_right

	def_plot plot_1x8_full   ""   "%d3" 8 plot_full
	def_plot plot_1x8_left   ""   ""    8 plot_left
	def_plot plot_1x8_right  ""   ""    8 plot_right

	.macro erase_full
	movem.l	(%a0),%d0-%d3	// 44: fetch 32x4-bit pixels (16 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1, pixels 0..15
	and.l	(%a1)+,%d1	// 14: mask planes 2&3, pixels 0..15
	addq	#8,%a1		//  8: skip unused data
	and.l	(%a1),%d2	// 14: mask planes 0&1, pixels 16..31
	and.l	(%a1)+,%d3	// 14: mask planes 2&3, pixels 16..31
	addq	#8,%a1		//  8: skip unused data
	movem.l %d0-%d3,(%a0)	// 40: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	.macro erase_left
	movem.l	(%a0),%d0-%d1	// 28: fetch 16x4-bit pixels (8 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1
	and.l	(%a1)+,%d1	// 14: mask planes 2&3
	lea	20(%a1),%a1	//  8: skip unused mask+data
	movem.l %d0-%d1,(%a0)	// 24: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	.macro erase_right
	movem.l	(%a0), %d0-%d1	// 28: fetch 16x4-bit pixels (8 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1
	and.l	(%a1)+,%d1	// 14: mask planes 2&3
	lea	20(%a1), %a1	//  8: skip unused mask+data
	movem.l	%d0-%d1, (%a0)	// 24: store right 16 pixels
	lea	160(%a0), %a0	//  8: next screen row
	.endm

	def_plot erase_1x1_full  ""   "%d3" 1 erase_full
	def_plot erase_1x1_left  ""   ""    1 erase_left
	def_plot erase_1x1_right "12" ""    1 erase_right

	def_plot erase_1x3_full  ""   "%d3" 3 erase_full
	def_plot erase_1x3_left  ""   ""    3 erase_left
	def_plot erase_1x3_right "12" ""    3 erase_right

	def_plot erase_1x4_full  ""   "%d3" 4 erase_full
	def_plot erase_1x4_left  ""   ""    4 erase_left
	def_plot erase_1x4_right "12" ""    4 erase_right

	def_plot erase_1x6_full  ""   "%d3" 6 erase_full
	def_plot erase_1x6_left  ""   ""    6 erase_left
	def_plot erase_1x6_right "12" ""    6 erase_right

	def_plot erase_1x8_full  ""   "%d3" 8 erase_full
	def_plot erase_1x8_left  ""   ""    8 erase_left
	def_plot erase_1x8_right "12" ""    8 erase_right

//   +8 -> data   [m0 d00 d01 m1 d10 d11 m2 d20 d21]
// sp+4 -> screen          [s00 s01 s10 s11 s20 s21]
// sp+0 -> ra
	.globl plot_2x6_full
plot_2x6_full:
	move.l	4(%sp), %a0
	move.l	8(%sp), %a1
	movem.l	%d2-%d5, -(%sp)
	.rep 6
	movem.l	(%a0),%d0-%d5	// 60: fetch 48x4-bit pixels (24 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1, pixels 0..15
	and.l	(%a1)+,%d1	// 14: mask planes 2&3, pixels 0..15
	or.l	(%a1)+,%d0	// 14: merge data planes 0&1, pix 0..15
	or.l	(%a1)+,%d1	// 14: merge data planes 2&3, pix 0..15
	and.l	(%a1),%d2	// 14: mask planes 0&1, pixels 16..31
	and.l	(%a1)+,%d3	// 14: mask planes 2&3, pixels 16..31
	or.l	(%a1)+,%d2	// 14: merge data planes 0&1, pix 16..31
	or.l	(%a1)+,%d3	// 14: merge data planes 2&3, pix 16..31
	and.l	(%a1),%d4	// 14: mask planes 0&1, pixels 32..47
	and.l	(%a1)+,%d5	// 14: mask planes 2&3, pixels 32..47
	or.l	(%a1)+,%d4	// 14: merge data planes 0&1, pix 32..47
	or.l	(%a1)+,%d5	// 14: merge data planes 2&3, pix 32..47
	movem.l %d0-%d5,(%a0) 	// 56: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endr
	movem.l	(%sp)+, %d2-%d5
	rts

//   +8 -> data   [m0 m1 m2 d00 d01 d10 d11 d20 d21]
// sp+4 -> screen          [s00 s01 s10 s11 s20 s21]
// sp+0 -> ra
	.globl	erase_2x6_full
erase_2x6_full:
	move.l	4(%sp), %a0
	move.l	8(%sp), %a1
	movem.l	%d2-%d5, -(%sp)
	.rep 6
	movem.l	(%a0),%d0-%d5	// 60: fetch 48x4-bit pixels (24 bytes)
	and.l	(%a1),%d0	// 14: mask planes 0&1, pixels 0..15
	and.l	(%a1)+,%d1	// 14: mask planes 2&3, pixels 0..15
	addq	#8,%a1
	and.l	(%a1),%d2	// 14: mask planes 0&1, pixels 16..31
	and.l	(%a1)+,%d3	// 14: mask planes 2&3, pixels 16..31
	addq	#8,%a1
	and.l	(%a1),%d4	// 14: mask planes 0&1, pixels 32..47
	and.l	(%a1)+,%d5	// 14: mask planes 2&3, pixels 32..47
	addq	#8,%a1
	movem.l %d0-%d5,(%a0) 	// 56: store pixels to screen
	lea	160(%a0), %a0	//  8: next screen row
	.endr
	movem.l	(%sp)+, %d2-%d5
	rts

	.extern	mterr
	.globl	plot_128_12_1bpp
plot_128_12_1bpp:
	move.l	4(%sp),%a1
	move.l	8(%sp),%a0
	.rept 12
	move.w	(%a0)+,0x00(%a1)
	move.w	(%a0)+,0x08(%a1)
	move.w	(%a0)+,0x10(%a1)
	move.w	(%a0)+,0x18(%a1)
	move.w	(%a0)+,0x20(%a1)
	move.w	(%a0)+,0x28(%a1)
	move.w	(%a0)+,0x30(%a1)
	move.w	(%a0)+,0x38(%a1)
	lea	16(%a0),%a0
	lea	160(%a1),%a1
	.endr
	rts

//  +12 -> nrows
//   +8 -> obj1 [m00 d00 d01 m01 d10 d11]
// sp+4 -> obj0 [m00 d00 d01 m01 d10 d11]
// sp+0 -> ra
	.globl spr_collide_full
spr_collide_full:
	move.l	4(%sp), %a0
	move.l	8(%sp), %a1
	move.l	12(%sp), %d1
	subq	#1,%d1		// for dbra
0:
	move.w	(%a0),%d0	// fetch player mask
	or.w	(%a1),%d0	// or-in object mask
	not.w	%d0
	bne.s	1f		// collision
	move.w	12(%a0),%d0	// fetch player mask
	or.w	12(%a1),%d0	// or-in object mask
	not.w	%d0
	bne.s	1f		// collision
	lea	24(%a0),%a0	// next player row
	lea	24(%a1),%a1	// next object row
	dbra	%d1,0b
1:
	rts

//  +12 -> nrows
//   +8 -> obj1 [m00 d00 d01 m01 d10 d11]
// sp+4 -> obj0 [m00 d00 d01 m01 d10 d11]
// sp+0 -> ra
	.globl spr_collide_half
spr_collide_half:
	move.l	4(%sp), %a0
	move.l	8(%sp), %a1
	move.l	12(%sp), %d1
	subq	#1,%d1		// for dbra
0:
	move.w	(%a0),%d0	// fetch player mask
	or.w	(%a1),%d0	// or-in object mask
	not.w	%d0
	bne.s	1f		// collision
	lea	24(%a0),%a0	// next player row
	lea	24(%a1),%a1	// next object row
	dbra	%d1,0b
1:
	rts

