/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.cpu	68030

	.globl	icache_flush
icache_flush:
	movec	%cacr,%d0
	or.l	#0x008,%d0		// Clear I-Cache
	movec	%d0,%cacr
	rts

