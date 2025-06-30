/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .rodata
	.align	2

	.globl	res_strtab
res_strtab:
	.incbin "gen/strings.bin"

#include "elfattr.S"
