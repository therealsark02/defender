
#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld

GITREV = $(shell git rev-parse --verify HEAD | cut -c1-8 | tr '[a-z]' '[A-Z]')
BUILDDATE = "$(shell date '+%d %b %Y')"

CFLAGS = -DM68000 -m68000
CFLAGS += -O3 -Wall -Werror -Wuninitialized
CFLAGS += -MMD
CFLAGS += -fno-builtin -static -ffixed-a6 -fomit-frame-pointer
CFLAGS += -Iinclude -Igen -Iliblzg/src/include
CFLAGS += -DGITREV=$(GITREV)
CFLAGS += -DBUILDDATE=$(BUILDDATE)

LDFLAGS = -nostartfiles -nodefaultlibs -nostdlib -static

OBJS = $(addprefix obj/, \
	start.o astro.o atari_font.o attr_defend.o attr_help.o attr_hof.o \
	attr_ledret.o attr_logo.o attr_ramtest.o attract.o bgalt.o blips.o \
	change_dir.o cmos.o collide.o color.o convert.o data.o defn_font.o \
	early_irq.o exec.o expl.o fireball.o gemdos.o gexec.o gitrev.o \
	grdata.o ground.o grplot.o hyper.o irq.o irq_thread.o \
	lander.o laser.o lib.o lproc.o lzg_api.o lzgmini_68k.o main.o mess.o \
	mkmterr.o mksprites.o mterr.o obj.o os_hooks.o panic.o panic_handler.o \
	panic_prf.o player.o plend.o plot.o plstrt.o prdisp.o probe.o proc.o \
	ramtest.o rand.o raster.o raster_irq.o render_char.o sbomb.o scanner.o \
	schitzo.o screen.o setup.o sfxplay.o shell.o sinit.o sound.o \
	sound_dma.o sound_psg.o stars.o strings.o swarmer.o swtch.o \
	sys.o tdisp.o terblo.o thread.o thrust.o tie.o timers.o tunes.o ufo.o \
	vbi_irq.o xcpt.o \
	)

all: defender.st

RELEASE_FILES = \
	defender.tos gen/dmasfx.bin gen/psgsfx.bin README LICENSE HOWTO

defender.st: tools/mkrel boot/boot.bin $(RELEASE_FILES)
	tools/mkrel boot/boot.bin $(RELEASE_FILES) defender.st defender.zip

boot/boot.bin:
	make -C boot

ifeq (,$(findstring m68k-elf-gcc,$(CC)))
defender.tos: $(OBJS)
	$(CC) $(LDFLAGS) -s -o $@ $^ -lgcc
	cp $@ gen
else
defender.tos: defender tools/mkprog/mkprog
	tools/mkprog/mkprog defender -o $@
	cp $@ gen

tools/mkprog/mkprog:
	make -C tools/mkprog

defender: $(OBJS)
	$(CC) $(LDFLAGS) -r -Tdefender.lds -o $@ $^ -lgcc
endif

obj/%.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $<

obj/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj/grdata.o: gen/grdata.bin
obj/mterr.o: gen/mterr.bin

gen/grdata.bin gen/mterr.bin: tools/mkground/mkground
	tools/mkground/mkground gen/grdata.bin gen/mterr.bin

tools/mkground/mkground:   
	make -C tools/mkground

obj/lzgmini_68k.o: gen/lzgmini_68k.S
	$(CC) -c $(CFLAGS) -o $@ $<

gen/lzgmini_68k.S: gen/.dir liblzg/src/extra/lzgmini_68k.s
	tools/mklzgmini/mklzgmini liblzg/src/extra/lzgmini_68k.s $@

$(OBJS): obj/.dir gen/asm_gd.h gen/str_res.h

gen/asm_gd.h: gen/.dir include/gd.h include/proc.h include/obj.h
	make -C tools/asm_gd

strings.bin gen/str_res.h: strings.res tools/mkstrtab/mkstrtab
	tools/mkstrtab/mkstrtab strings.res gen/str_res.h gen/strings.bin

tools/mkstrtab/mkstrtab:
	make -C tools/mkstrtab

gen/dmasfx.bin: gen/.dir tools/mksfx/mksfx liblzg/src/tools/lzg
	tools/mkdmasfx

gen/psgsfx.bin: gen/.dir tools/mksfx/mksfx liblzg/src/tools/lzg
	tools/mkpsgsfx

tools/mksfx/mksfx:
	make -C tools/mksfx

liblzg/src/tools/lzg: liblzg/src/lib/liblzg.a
	make -C liblzg/src

liblzg/src/lib/liblzg.a:
	make -C liblzg/src/lib

%/.dir:
	mkdir -p $(dir $@)
	touch $@

.PHONY: clean
clean:
	@make -C boot clean
	@make -C tools/asm_gd clean
	@make -C tools/mkprog clean
	@make -C tools/mkstrtab clean
	@make -C tools/mkground clean
	@make -C tools/mksfx clean
	@rm -f defender defender.tos defender.st defender.zip
	@rm -rf gen obj

-include obj/*.d

