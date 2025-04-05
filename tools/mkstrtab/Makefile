
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

CFLAGS = -MMD -O3 -Wall -Werror -Wuninitialized

all: mkstrtab

mkstrtab: main.o sbuf.o symtab.o
	gcc $(CFLAGS) -o $@ $^

.c.o:
	gcc -c $(CFLAGS) $<

clean:
	@rm -f *.o *.d mkstrtab

-include *.d

