/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .text

	.globl	LZG_DecodedSize
LZG_DecodedSize:
	move.l	4(%sp),%a0
	move.l	8(%sp),%d0
	jsr	_LZG_DecodedSize
	move.l	%d1,%d0
	rts

	.globl	LZG_Decode
LZG_Decode:
	move.l	%d2,-(%sp)
	move.l	8(%sp),%a0
	move.l	12(%sp),%d0
	move.l	16(%sp),%a1
	move.l	20(%sp),%d1
	jsr	_LZG_Decode
	move.l	%d2,%d0
	move.l	(%sp)+,%d2
	rts

#include "elfattr.S"
