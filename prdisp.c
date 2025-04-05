/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Player ship display.
 */

#include "gd.h"
#include "sprites.h"
#include "screen.h"
#include "plot.h"
#include "prdisp.h"

void prdisp(void)
{
    uint16_t s, x, y, idx;
    const sprpat_t *spr;
    const uint16_t *src;
    uint16_t *dst;

    if (gd->status & ST_PLNOCOLL) {
        return;
    }
    y = gd->playc;
    if (y <= gd->pminy || y > gd->pmaxy) {
        return;
    }
    x = gd->plascrx = gd->nplascrx;
    y = gd->playc = gd->nplayc;
    gd->plaxc = gd->nplaxc;
    gd->pladir = gd->nplad;
    if (gd->pladir >= 0) {
        idx = P_PLYRR;
        x -= 8;
    } else {
        idx = P_PLYRL;
    }
    if (gd->pia21 & PIA21_THRUST) {
        idx += N_PTHRUST;
    }
    spr = &sprtab[idx + gd->pridx];
    dst = (uint16_t *)SCRPTR(x, y);
    s = x & 0xf;
    src = spr->pat[s];
    if (gd->opldst) {
        erase_2x6_full(gd->opldst, gd->oplsrc);
    }
    plot_2x6_full(dst, src);
    gd->opldst = dst;
    gd->oplsrc = src;
}

void plthrproc(void)
{
    gd->pridx = gd->pridx + 1;
    if (gd->pridx == N_PTHRUST) {
        gd->pridx = 0;
    }
    gd->fbidx = gd->fbidx + 1;
    if (gd->fbidx == N_PFIREBALL) {
        gd->fbidx = 0;
    }
    sleep(plthrproc, 4);
}
