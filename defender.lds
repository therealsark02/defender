/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

SECTIONS
{
	.text : {
		*start.o(.text);
		*(.text*);
		. = ALIGN(4);
	}
	.data : {
		_data = .;
		*(.data*);
		*(.rodata*);
		. = ALIGN(4);
		_edata = .;
	}
	.bss : {
		. = ALIGN(4);
		_bss = .;
		*(.bss);
		*(COMMON);
		. = ALIGN(4);
		_end = ALIGN(4096);
	}
}
