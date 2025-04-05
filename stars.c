/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Stars display.
 */

#include "gd.h"
#include "rand.h"
#include "screen.h"
#include "stars.h"

star_t smap[SNUM];

static uint32_t smask[16];
static uint32_t sdata[8][16][2];

static void star_plot(star_t *stp, uint8_t px)
{
    uint16_t x = u8_to_screenx(stp->sx) | px;
    uint32_t *dst = (uint32_t *)SCRPTR(x, stp->sy);
    uint8_t shift = x & 0xf;
    uint32_t mask = smask[shift];
    
    dst[0] |= sdata[stp->scol][shift][0];
    dst[1] |= sdata[stp->scol][shift][1];
    stp->eradst = dst;
    stp->eramask = mask;
}

void stout(void)
{
    uint8_t A, itemp, px, rnd;
    uint16_t D, itemp16;
    star_t *stp;

    if (gd->status & ST_NOSTOBJS) {
        // stars, objects, shells output disabled
        return;
    }
    itemp16 = gd->bgl & 0xff80;
    D = ((gd->bglx & 0xff80) - itemp16) << 1;
    itemp = D >> 8;                             // STA ITEMP
    px = (gd->bgl & 0x40) ? 0 : 1;

    // erase all stars
    smap[0].eradst[0] &= smap[0].eramask;
    smap[0].eradst[1] &= smap[0].eramask;
    smap[1].eradst[0] &= smap[1].eramask;
    smap[1].eradst[1] &= smap[1].eramask;
    smap[2].eradst[0] &= smap[2].eramask;
    smap[2].eradst[1] &= smap[2].eramask;
    smap[3].eradst[0] &= smap[3].eramask;
    smap[3].eradst[1] &= smap[3].eramask;
    smap[4].eradst[0] &= smap[4].eramask;
    smap[4].eradst[1] &= smap[4].eramask;
    smap[5].eradst[0] &= smap[5].eramask;
    smap[5].eradst[1] &= smap[5].eramask;
    smap[6].eradst[0] &= smap[6].eramask;
    smap[6].eradst[1] &= smap[6].eramask;
    smap[7].eradst[0] &= smap[7].eramask;
    smap[7].eradst[1] &= smap[7].eramask;
    smap[8].eradst[0] &= smap[8].eramask;
    smap[8].eradst[1] &= smap[8].eramask;
    smap[9].eradst[0] &= smap[9].eramask;
    smap[9].eradst[1] &= smap[9].eramask;
    smap[10].eradst[0] &= smap[10].eramask;
    smap[10].eradst[1] &= smap[10].eramask;
    smap[11].eradst[0] &= smap[11].eramask;
    smap[11].eradst[1] &= smap[11].eramask;
    smap[12].eradst[0] &= smap[12].eramask;
    smap[12].eradst[1] &= smap[12].eramask;
    smap[13].eradst[0] &= smap[13].eramask;
    smap[13].eradst[1] &= smap[13].eramask;
    smap[14].eradst[0] &= smap[14].eramask;
    smap[14].eradst[1] &= smap[14].eramask;
    smap[15].eradst[0] &= smap[15].eramask;
    smap[15].eradst[1] &= smap[15].eramask;
// STOUT1
    for (stp = &smap[0]; stp != &smap[SNUM]; stp++) {
        A = stp->sx + itemp;
        if (A >= screenx_to_u8(304)) {
            if (A > 0xc0) {
                A = screenx_to_u8(303);
            } else {
                A = screenx_to_u8(16);
            }
        } else if (A < screenx_to_u8(16)) {
            A = screenx_to_u8(303);
        }
        stp->sx = A;
        star_plot(stp, px);
    }
    
    // star blink
    stp = &smap[(gd->seed >> 2) & (SNUM - 1)];
    stp->scol = (stp->scol + 0x1) & 0x7;
    rnd = gd->seed;
    if ((rnd & 1) == 0) {
        if (rnd > 0x98) {
            rnd -= 0x84;
        } else if (rnd < 8) {   // extra bit for ST
            rnd += 0x14;
        }
        // erase star
        stp->eradst[0] &= stp->eramask;
        stp->eradst[1] &= stp->eramask;
        stp->sx = rnd;
        if (gd->status & ST_NOTERRAIN) {
            stp->sy = (gd->lseed & 0x3f) * 3 + YMIN;
        }
    }
}

void stinit(void)
{
    uint8_t i, scol;

    gd->strcnt = SNUM;
    scol = 0;
    for (i = 0; i < SNUM; i++) {
        smap[i].sx = randrng(32, 155);
        smap[i].sy = randrng(YMIN + 1, 168);
        smap[i].scol = scol;
        smap[i].eradst = (uint32_t *)gd->vid_base;
        scol = (scol + 0x1) & 0x7;
    }
}

void stars_init(void)
{
    uint8_t i, scol, pcol;
    uint16_t w0, w1, w2, w3;
    uint32_t d0, d1;

    // and-masks
    for (i = 0; i < 16; i++) {
        smask[i] = ~(0x80008000 >> i);
    }

    // color or-masks
    for (scol = 0; scol < 8; scol++) {
        pcol = (scol == 7) ? 8 : scol;
        w0 = (pcol & 1) ? 0x8000 : 0;
        w1 = (pcol & 2) ? 0x8000 : 0;
        w2 = (pcol & 4) ? 0x8000 : 0;
        w3 = (pcol & 8) ? 0x8000 : 0;
        d0 = (w0 << 16) | w1;
        d1 = (w2 << 16) | w3;
        for (i = 0; i < 16; i++) {
            sdata[scol][i][0] = d0;
            sdata[scol][i][1] = d1;
            d0 >>= 1;
            d1 >>= 1;
        }
    }
}
