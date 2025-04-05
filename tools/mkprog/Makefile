
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

CFLAGS = -MMD -O3 -Wall -Werror -Wuninitialized

all: mkprog

mkprog: main.o elf.o
	gcc $(CFLAGS) -o $@ $^

.c.o:
	gcc -c $(CFLAGS) $<

clean:
	@rm -f *.o *.d mkprog

-include *.d

