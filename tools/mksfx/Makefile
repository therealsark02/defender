
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

TOP = ../..

OBJS = main.o wav.o

CFLAGS = -O3 -Wall -Werror -Wuninitialized -MMD

mksfx: $(OBJS)
	gcc -o $@ $^ -lm

.c.o:
	gcc -c $(CFLAGS) -I$(TOP)/include $<

.PHONY: clean
clean:
	@rm -f mksfx $(OBJS) *.d

-include *.d

