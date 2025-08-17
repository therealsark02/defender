/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Game start: (fake) RAM Test.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "mess.h"
#include "attract.h"

void ramtest_scrinit(void);
void ramtest_step(void);

#define sleep(a, b) attr_sleep((a), (b))
#define sucide()    attr_sucide()

static void ramok(void)
{
    scrclr();
    pcram[1] = 0x7a;
    colr_apply();
    messf(0x28 * 2, 0x70, ID_INITIAL_TESTS_INDICATE);
    messf(0x40 * 2, 0x90, ID_UNIT_OK);
    sleep(amodes, 50 * 3);
}

static void ramtest_setscr(void)
{
    proc_t *p = gd->crproc;

    uint8_t shift = p->ramtest.x & 0xf;

    p->ramtest.ptr = (uint32_t *)SCRPTR(p->ramtest.x, 7);
    p->ramtest.colptr0 = col_to_planes[shift + 0];
    p->ramtest.colptr1 = col_to_planes[shift + 1];
    p->ramtest.mask = ~(0xc000c000 >> shift);
    p->ramtest.shift = shift;
}

#define RAMTEST_POSTWRITE_PAUSE 75

static void ramtest1(void);

static void waitchars(void)
{
    if (gd->chars_ready) {
        sleep(ramok, 1);
    } else {
        sleep(waitchars, 1);
    }
}

static void ramtest2(void)
{
    proc_t *p = gd->crproc;

    if (--p->ramtest.cnt == 0) {
        waitchars();
    } else {
        p->ramtest.x = 16;
        ramtest_setscr();
        sleep(ramtest1, 1);
    }
}

static void ramtest1(void)
{
    proc_t *p = gd->crproc;
    uint16_t i;

    for (i = 0; i < 2; i++) {
        ramtest_step();
        p->ramtest.x += 2;
        ramtest_setscr();
        if (p->ramtest.x == 304) {
            sleep(ramtest2, RAMTEST_POSTWRITE_PAUSE);
        }
    }
    sleep(ramtest1, 1);
}

void ramtest(void)
{
    proc_t *p = gd->crproc;
    uint8_t col;
    uint16_t i;

    pcram[0] = 0;
    col = 0xc0;
    for (i = 1; i < 16; i++) {
        pcram[i] = col;
        col = (col * 0xb5) >> 8;
    }
    colr_apply();
    ramtest_scrinit();
    p->ramtest.x = 16;
    ramtest_setscr();
    p->ramtest.cnt = 2;
    p->ramtest.a = 0;
    p->ramtest.b = 0;
    sleep(ramtest1, 1);
}
