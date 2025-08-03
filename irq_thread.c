/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * IRQ Thread. This is scheduled from the mid-screen raster-interrupt and
 * and vertical blank interrupt. In the original game its an actual IRQ
 * handler, but here it's essentially a high-priority thread. This code
 * runs the draw/erase code for all the visible game elements.
 */

#include "gd.h"
#include "screen.h"
#include "ground.h"
#include "prdisp.h"
#include "obj.h"
#include "player.h"
#include "stars.h"
#include "shell.h"
#include "color.h"
#include "sysvars.h"
#include "irq_thread.h"

static void grid(void)
{
    uint16_t *p;
    uint8_t i;
    const int plane = 1;

    if (gd->want_grid) {
        for (p = (uint16_t *)gd->vid_base + plane; p < (uint16_t *)phystop; p += 4) {
            *p |= 0x8000;
        }
        for (p = (uint16_t *)gd->vid_base + 80 * 9 + plane; p < (uint16_t *)phystop; p += (16 * 80)) {
            for (i = 0; i < 80; i += 4) {
                p[i] = 0xffff;
            }
        }
        gd->grid_on = 1;
    } else if (gd->grid_on) {
        for (p = (uint16_t *)gd->vid_base + plane; p < (uint16_t *)phystop; p += 4) {
            *p &= 0x7fff;
        }
        for (p = (uint16_t *)gd->vid_base + 80 * 9 + plane; p < (uint16_t *)phystop; p += (16 * 80)) {
            for (i = 0; i < 80; i += 4) {
                p[i] = 0;
            }
        }
        gd->grid_on = 0;
    }
}

void irq_thread(void)
{
    uint8_t want_timing = gd->want_timing;

    if (gd->curirq == 0x99) {
        // process the lower half of the screen
        if (!gd->iflg) {
            // upper half needs to go first
            return;
        }
        gd->iflg = 0;
        if (want_timing) {
            eorcolor(0x770);
        }
        ++gd->timer;
        // color mapping
        colr_apply();
        if (!(gd->status & ST_NOTERRAIN)) {
            bgout();
        }
        gd->pminy = gd->pmaxy;
        gd->pmaxy = 255;
        prdisp();
        oproc();
        velo();
        grid();
        if (want_timing) {
            eorcolor(0x770);
        }
    } else if (gd->iflg == 0) {
        // process the upper half of the screen
        if (gd->iflg) {
            // upper half needs to go first
            return;
        }
        ++gd->iflg;
        if (want_timing) {
            eorcolor(0x707);
        }
        sndseq();           // sounds and switches
        player();           // player movement
        stout();            // output stars
        gd->pminy = 0;
        gd->pmaxy = SCREEN_HALF_Y - 8;
        oproc();            // output objects
        prdisp();           // output player
        shell();            // output shells
        if (want_timing) {
            eorcolor(0x707);
        }
    }
}
