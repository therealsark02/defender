
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

CFLAGS = -O3 -Wall -Wuninitialized -Werror -MMD

OBJS = main.o data.o ground.o

mkground: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	@rm -f *.d $(OBJS) mkground

-include *.d
