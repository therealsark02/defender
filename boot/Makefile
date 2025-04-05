
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

TOP = ..

CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NM = $(CROSS_COMPILE)nm

CFLAGS = -DM68000 -m68000
CFLAGS += --param min-pagesize=0
CFLAGS += -I$(TOP)/include

CFLAGS += -fno-PIC -nostdlib -ffunction-sections -fdata-sections

LDFLAGS += -Wl,--relocatable -Wl,--strip-all -Wl,--entry=_start

all: boot.bin

boot.bin: boot
	$(OBJCOPY) -O binary $^ $@

boot: boot.o
	$(CC) $(CFLAGS) $^ -o $@

.S.o:
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f boot *.bin *.o
