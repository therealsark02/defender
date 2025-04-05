/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Panic handler. Something unexpected happened.
 */

#include <stdarg.h>
#include "gd.h"
#include "color.h"
#include "screen.h"
#include "vectors.h"
#include "panic.h"

uint16_t panic_vector;
uint16_t panic_code;
uint16_t panic_instr;
uint16_t panic_sr;
uint32_t panic_badaddr;
uint32_t panic_pc;
uint32_t panic_aregs[8];
uint32_t panic_dregs[8];
extern char _start[];
extern char _ebss[];
extern const char git_rev[];

#define EXNAMES_GENERATOR(a, b) [V_##a >> 2] = #a,
static const char *exnames[N_VECTORS] = {
    "PANIC",
    VECTOR_GENERATOR(EXNAMES_GENERATOR)
};

static void panic_headline(void)
{
    const char *exname;

    panic_printf("PROGRAM HALTED (GIT %s)\n\n", git_rev);
    exname = ((panic_vector) < 0x140) ? exnames[panic_vector >> 2] : NULL;
    if (exname != NULL) {
        panic_printf("%S", exname);
    } else {
        panic_printf("EXCEPTION %w", panic_vector);
    }
    panic_printf(" @ PC:%l, SR:%w\n", panic_pc, panic_sr);
    if (panic_vector == V_BUS_ERROR || panic_vector == V_ADDRESS_ERROR) {
        panic_printf("%s %s %s @ ADDR:%l\n",
            (panic_code & 0x4) ? "SUPER" : "USER",
            (panic_code & 0x8) ? "INST" : "DATA",
            (panic_code & 0x10) ? "READ" : "WRITE",
            panic_badaddr);
    }
}

static void panic_dump(void)
{
    uint16_t *sp;
    uint8_t r;

    pcram[0] = 0x03;
    pcram[1] = 0x65;
    colr_apply();
    scrclr();

    gd->panic_row = 0;
    gd->panic_col = 0;
    panic_headline();
    gd->panic_row += 2;
    gd->panic_col = 0;
    panic_printf("TEXT @ %l\n", _start);
    for (r = 0; r < 16; r++) {
        if ((r & 3) == 0) {
            panic_printf("%c%n:", (r < 8) ? 'D' : 'A', r & 7);
        }
        panic_printf("%l", (r < 8) ? panic_dregs[r] : panic_aregs[r - 8]);
        if ((r & 3) == 3) {
            gd->panic_col = 0;
            gd->panic_row += (r == 7) ? 2 : 1;
        } else {
            ++gd->panic_col;
        }
    }
    sp = (uint16_t *)panic_aregs[7];
    if (((uint32_t)sp & 1) ||
        (char *)sp > _end ||
        (char *)sp < _ebss) {
        return;
    }
    gd->panic_row += 2;
    for (r = 0; (char *)sp < _end && r < 40; r++) {
        if ((r & 3) == 0) {
            panic_printf("%a:", sp);
        }
        panic_printf("%w", *sp++);
        ++gd->panic_col;
        if ((r & 3) == 3) {
            ++gd->panic_row;
            gd->panic_col = 0;
        }
    }
}

static void panic_collect(void)
{
    uint16_t *a7 = (uint16_t *)panic_aregs[7];

    panic_vector = *a7++;
    if (panic_vector == V_BUS_ERROR || panic_vector == V_ADDRESS_ERROR) {
        panic_code = *a7++;
        panic_badaddr = *(uint32_t *)a7;
        a7 += 2;
        panic_instr = *a7++;
    }
    panic_sr = *a7++;
    panic_pc = *(uint32_t *)a7;
}

void panic_handler(void)
{
    panic_collect();
    panic_dump();
    for (;;);
}

