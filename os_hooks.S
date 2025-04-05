/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "sysvars.h"

	.section .text

	.globl	invoke_os_hooks
	// sp->[0:ret,4:func]
invoke_os_hooks:
	move.l	4(%sp),%a0	// os vbi vector
	movem.l	%d2-%d7/%a2-%a6,-(%sp)
	// call OS vbl entry
	tst.w	_longframe
	beq.s	0f
	clr.w	-(%sp)		// format word
0:
	pea	0f(%pc)		// return address
	move.w	%sr,-(%sp)	// sr
	jmp	(%a0)		// invoke os vbi handler
0:
	// Timer C work chain (@200Hz/4 = 50Hz)
	addq.l	#4,_hz_200
	move.w	_timr_ms,-(%sp)
	movea.l	etv_timer,%a0
	jsr	(%a0)
	addq.w	#2,%sp
	movem.l	(%sp)+,%d2-%d7/%a2-%a6
	rts
