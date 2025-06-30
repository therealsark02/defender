/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .rodata
	.align	2

	.globl	mterr
mterr:
	.incbin "gen/mterr.bin"

#include "elfattr.S"
