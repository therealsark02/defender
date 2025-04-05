/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Blips. A "blip" is the 2x2 square that appears on the scanner.
 * Each enemy object type has a specific blip.
 */

#include "gd.h"
#include "obj.h"
#include "scanner.h"
#include "screen.h"
#include "sprites.h"
#include "plot.h"
#include "blips.h"

#define BLIP_ENTRY_GEN(a, b) { 0x##a, 0x##b },
static const uint8_t blipcols[B__NUM][2] = {
    BLIP_GENERATOR(BLIP_ENTRY_GEN)
};

blip_t bliptab[B__NUM];

static uint32_t *blips_erase_tab[NOBJS + 1]; // +1 for NULL
static uint32_t **blips_erase_ptr;
static uint32_t *blips_player_erase;

const uint32_t blipmask[8] = {
    0x3fff3fff, 0xcfffcfff,
    0xf3fff3ff, 0xfcfffcff,
    0xff3fff3f, 0xffcfffcf,
    0xfff3fff3, 0xfffcfffc
};

void blips_init(void)
{
    uint8_t i, pcol, shift;
    uint16_t w0, w1, w2, w3;
    blip_t *bp;

    // color or-masks
    for (i = 0; i < B__NUM; i++) {
        bp = &bliptab[i];
        pcol = blipcols[i][0] >> 4;
        w0  = (pcol & 1) ? 0x8000 : 0;
        w1  = (pcol & 2) ? 0x8000 : 0;
        w2  = (pcol & 4) ? 0x8000 : 0;
        w3  = (pcol & 8) ? 0x8000 : 0;
        pcol = blipcols[i][0] & 0xf;
        w0 |= (pcol & 1) ? 0x4000 : 0;
        w1 |= (pcol & 2) ? 0x4000 : 0;
        w2 |= (pcol & 4) ? 0x4000 : 0;
        w3 |= (pcol & 8) ? 0x4000 : 0;
        bp->pat[0][0] = (w0 << 16) | w1;
        bp->pat[0][1] = (w2 << 16) | w3;
        pcol = blipcols[i][1] >> 4;
        w0  = (pcol & 1) ? 0x8000 : 0;
        w1  = (pcol & 2) ? 0x8000 : 0;
        w2  = (pcol & 4) ? 0x8000 : 0;
        w3  = (pcol & 8) ? 0x8000 : 0;
        pcol = blipcols[i][1] & 0xf;
        w0 |= (pcol & 1) ? 0x4000 : 0;
        w1 |= (pcol & 2) ? 0x4000 : 0;
        w2 |= (pcol & 4) ? 0x4000 : 0;
        w3 |= (pcol & 8) ? 0x4000 : 0;
        bp->pat[0][2] = (w0 << 16) | w1;
        bp->pat[0][3] = (w2 << 16) | w3;
        for (shift = 1; shift < 8; shift++) {
            bp->pat[shift][0] = bp->pat[shift - 1][0] >> 2;
            bp->pat[shift][1] = bp->pat[shift - 1][1] >> 2;
            bp->pat[shift][2] = bp->pat[shift - 1][2] >> 2;
            bp->pat[shift][3] = bp->pat[shift - 1][3] >> 2;
        }
    }
}

void blips_erase(void)
{
    uint32_t **ep;
    uint32_t *dst;

    ep = blips_erase_tab;
    dst = *ep++;
    while (dst != NULL) {
        dst[0] = 0;
        dst[1] = 0;
        dst[40] = 0;
        dst[41] = 0;
        dst = *ep++;
    }
    dst = blips_player_erase;
    if (dst != NULL) {
        dst[0] = 0;
        dst[1] = 0;
        dst[2] = 0;
        dst[3] = 0;
        dst[40] = 0;
        dst[41] = 0;
        dst[42] = 0;
        dst[43] = 0;
        dst[80] = 0;
        dst[81] = 0;
        dst[82] = 0;
        dst[83] = 0;
    }
}

static void blips_plot_objs(const obj_t *obj, uint16_t offsx)
{
    const uint32_t *pat;
    const blip_t *bp;
    uint16_t ox, oy;
    uint32_t mask;
    uint32_t *dst;
    uint32_t **ep;
    uint8_t shift;

    ep = blips_erase_ptr;
    for (; obj != NULL; obj = obj->olink) {
        bp = obj->objcol;
        if (bp == NULL) {
            continue;
        }
        ox = SCANER_X + (((uint16_t)(obj->ox16 - offsx) >> 9) & 0x7e);
        oy = SCANER_Y + (obj->oy16 >> 11);
        dst = (uint32_t *)SCRPTR(ox, oy);
        *ep++ = dst;
        shift = (ox & 0xf) >> 1;
        pat = bp->pat[shift];
        mask = blipmask[shift];
        dst[0] = (dst[0] & mask) | pat[0];
        dst[1] = (dst[1] & mask) | pat[1];
        dst[40] = (dst[40] & mask) | pat[2];
        dst[41] = (dst[41] & mask) | pat[3];
    }
    blips_erase_ptr = ep;
}

static void blips_plot_player(void)
{
    const sprpat_t *spr = &sprtab[P_PLAYERBLIP];
    const uint16_t *src;
    uint16_t ox, oy;
    uint32_t *dst;

    ox = (0x4b << 1) - 1 + ((gd->plaxc >> 3) & 0xfe);
    oy = (gd->playc >> 3) + SCANER_Y - 1;
    dst = (uint32_t *)SCRPTR(ox, oy);
    blips_player_erase = dst;
    src = spr->pat[ox & 0xf];
    plot_1x3_full((uint16_t *)dst, src);
}

void blips_plot(uint16_t offsx)
{
    blips_erase_ptr = blips_erase_tab;
    blips_plot_objs(gd->optr, offsx);
    blips_plot_objs(gd->iptr, offsx);
    if (blips_erase_ptr > &blips_erase_tab[NOBJS]) {
        panic(0xe1e100e1, blips_erase_ptr, &blips_erase_tab[NOBJS]);
    }
    *blips_erase_ptr++ = NULL;
    blips_plot_player();
}

