/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Attract mode entry.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "attract.h"

#define sleep(a, b) attr_sleep((a), (b))
#define sucide()    attr_sucide()

void attr_end_evbi(void)
{
    gncide();
    sucide();
}

void attr_wait_end(void)
{
    if (gd->emain_finished) {
        sleep(attr_end_evbi, 1);
    } else {
        sleep(attr_wait_end, 1);
    }
}

void attr_resume(void)
{
    gd->startup = 0;
    mkproc(colr, AMTYPE);
    mkproc(tiecol, AMTYPE);
    mkproc(cbomb, AMTYPE);
    pcram[12] = 0x3f;
    mkproc(defend_resume, AMTYPE);
}

void amodes(void)
{
    gncide();
    gd->status = 0xfb;
    scrclr();
    crinit();
    mkproc(colr, AMTYPE);
    mkproc(tiecol, AMTYPE);
    pcram[12] = 0x3f;
    logo();
}

