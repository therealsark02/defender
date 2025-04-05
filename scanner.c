/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Scanner display.
 */

#include "gd.h"
#include "screen.h"
#include "plot.h"
#include "obj.h"
#include "blips.h"
#include "shell.h"
#include "mkmterr.h"
#include "mterr.h"
#include "scanner.h"
#include "grdata.h"

static void scp2(void)
{
    uint8_t mtx, mts, mti;
    const uint16_t *src;
    uint16_t osx, nx;
    uint16_t *dst;

    // erase old bozos and player
    blips_erase();

    // establish world-coord of left-edge of the scanner
    osx = gd->bgl - (0x8000 - 150 * 32);

    // draw mini-terrain, if enabled
    if (!(gd->status & ST_NOTERRAIN)) {
        nx = ((osx >> 5) + 18) & (GROUND_WIDTH - 1);
        mtx = nx >> 5;  // 0..2047 -> 0..63
        mts = mtx & 7;  // 0..63 -> 0..7
        mti = mtx >> 3;
        if (mts >= 8) panic(0xdeadf00d);
        src = mterrpat[mts] + mti;
        dst = (uint16_t *)SCRPTR(SCANER_X, SCANER_Y + 0x20 - MTERRH) + 3;
        plot_128_12_1bpp(dst, src);
    }

    // draw bezel
    dst = (uint16_t *)SCRPTR(0x4c*2, SCANER_Y + 1);
    dst[0] = 0x0080; dst[4] = 0x0100;
    dst[3] = 0x0080; dst[7] = 0x0100;
    dst += 80;
    dst[0] = 0x0080; dst[4] = 0x0100;
    dst[3] = 0x0080; dst[7] = 0x0100;
    dst = (uint16_t *)SCRPTR(0x4c*2, SCANER_Y + 0x20 - 2);
    dst[0] |= 0x0080; dst[4] |= 0x0100;
    dst[1] &= 0xff7f; dst[5] &= 0xff7f;
    dst[2] &= 0xff7f; dst[6] &= 0xff7f;
    dst[3] |= 0x0080; dst[7] |= 0x0100;
    dst += 80;
    dst[0] |= 0x0080; dst[4] |= 0x0100;
    dst[1] &= 0xff7f; dst[5] &= 0xff7f;
    dst[2] &= 0xff7f; dst[6] &= 0xff7f;
    dst[3] |= 0x0080; dst[7] |= 0x0100;

    // output bozos and player
    blips_plot(osx);

    sleep(scproc, 4);
}

static void scp1(void)
{
    oscan();
    shscan();
    sleep(scp2, 2);
}

void scproc(void)
{
    iscan();
    sleep(scp1, 2);
}
