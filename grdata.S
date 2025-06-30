/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .rodata
	.align	2
	.globl	grdata
grdata:
	.incbin "gen/grdata.bin"

#include "elfattr.S"
