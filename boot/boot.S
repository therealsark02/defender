/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "sysvars.h"

	.globl	_start
	.section .text
	bra.s	_start			// 00: BRA.S
	.byte	0x00, 0x00, 0x00	// 02: OEM
	.byte	0x00, 0x00, 0x00	// 05: OEM
	.byte	0xde, 0xfe, 0xed	// 08: SERIAL
	.byte	0x00, 0x02		// 0b: BPS
	.byte	0x02			// 0d: SPC
	.byte	0x01, 0x00		// 0e: RES
	.byte	0x02			// 10: NFATS
	.byte	0x70, 0x00		// 11: NDIRS
	.byte	0xa0, 0x05		// 13: NSECTS
	.byte	0xf9			// 15: MEDIA
	.byte	0x03, 0x00		// 16: SPF
	.byte	0x09, 0x00		// 18: SPT
	.byte	0x02, 0x00		// 1a: NSIDES
	.byte	0x00, 0x00		// 1c: NHID

_start:
	move.l	#0x5f4d4348,%d0		// _MCH
	bsr	get_cookie
	beq.s	st_vid
	cmp.l	#0x00030000,%d0		// Falcon?
	bne.s	st_vid
falcon_vid:
	// check for mono screen
	move.w	#89,-(%sp)		// MON_TYPE
	trap	#14
	tst.w	%d0
	beq.s	vid_bad
	// setup screen
	move.w	#0xa2,%d1		// base vmode
	cmp.w	#2,%d0			// VGA?
	bne.s	0f			// no
	or.w	#0x110,%d1		// +VGA bits
0:
	move.w	%d1,-(%sp)
	move.w	#3,-(%sp)
	bra.s	setscr
st_vid:
	// check for mono screen
	move.w	#4,-(%sp)		// GETREZ
	trap	#14
	cmp.w	#2,%d0			// mono?
	bne.s	0f
vid_bad:
	pea	fail_mono_msg(%pc)
	bra	fail
0:
	// setup screen
	clr.w	-(%sp)			// low res
setscr:
	move.l	#-1,-(%sp)		// no change physaddr
	move.l	#-1,-(%sp)		// no change logaddr
	move.w	#5,-(%sp)		// SETSCREEN
	trap	#14
	move.w	#37,-(%sp)		// WVBL
	trap	#14
	clr.w	-(%sp)			// black
	clr.w	-(%sp)			// color 0
	move.w	#7,-(%sp)		// SETCOLOR
	trap	#14
	move.w	#0x770,-(%sp)		// yellow
	move.w	#15,-(%sp)		// color 1
	move.w	#7,-(%sp)		// SETCOLOR
	trap	#14

	// memory check
	cmp.l	#0x100000,phystop	// phystop at least 1MB?
	bhs.s	mem_ok
	pea	fail_nomem_msg(%pc)
	bra	fail
mem_ok:
	// emit defender title
	lea	title_msg(%pc),%a0
	bsr	outstr

	// run defender
	pea	nul_str(%pc)
	pea	cmdline(%pc)
	pea	defender_path(%pc)
	clr.w	-(%sp)
	move.w	#0x4b,-(%sp)
	trap	#1
	bmi	diskfail
0:
	bra.s	0b

get_cookie:
	move.l	_p_cookies,%d1
	beq.s	2f			// no cookies
	move.l	%d1,%a0
	bra.s	1f
0:
	move.l	(%a0)+,%d2
	cmp.l	%d0,%d1			// found?
	beq.s	3f			// yes
1:
	move.l	(%a0)+,%d1
	bne.s	0b
2:
	// return with Z=1 (not found)
	rts
3:
	move.l	%d2,%d0
	// Z=0 (found)
	rts

outstr:
	move.l	%a0,-(%sp)
	move.w	#9,-(%sp)
	trap	#1
	addq	#6,%sp
	rts

diskfail:
	pea	fail_disk_err_msg(%pc)
fail:
	move.w	#9,-(%sp)
	trap	#1
0:
	bra.s	0b

defender_path:
	.string "DEFENDER.TOS"
nul_str:
	.byte	0
title_msg:
	.byte	27,'E',27,'b',15,27,'f'
	.byte	27,'Y',32+12,32+20-8
cmdline:
	.string	"D E F E N D E R"
fail_mono_msg:
	.byte	27,'E',27,'Y',32+12,32+40-17
	.string	"DEFENDER requires a color monitor."
fail_nomem_msg:
	.byte	27,'E',27,'Y',32+12,32+20-18
	.string	"DEFENDER requires at least 1MB RAM."
fail_disk_err_msg:
	.byte	27,'E',27,'Y',32+12,32+20-15
	.string	"File loading error."

#include "../elfattr.S"
