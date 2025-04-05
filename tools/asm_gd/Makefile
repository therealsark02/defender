
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

TOP = ../..

CFLAGS = -g -MMD -I$(TOP)/include -nostdinc -fno-builtin
$(TOP)/gen/asm_gd.h: asm_gd.o
	./dwparse.pl $< > $@

asm_gd.o: asm_gd.c
	$(CROSS_COMPILE)gcc -c $(CFLAGS) $<

clean:
	@rm -f asm_gd.o $(TOP)/gen/asm_gd.h *.d

-include *.d
