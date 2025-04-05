/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Convert a packed sprite image to a 16-bit shifted, expanded,
 * and masked planar format.
 */

#include "gd.h"
#include "data.h"
#include "sprpat.h"
#include "plot.h"
#include "convert.h"

#define CASE_ASSIGN_GEN(a, b) \
    spr->draw_##b = plot_1x##a##_##b; \
    spr->erase_##b = erase_1x##a##_##b;

#define CASE_GEN(a) \
    case a: \
        PLOT_LFR_GENERATOR(a, CASE_ASSIGN_GEN) \
        break;

void convert_pict(sprpat_t *spr, const pict_t *pict)
{
    uint16_t shift, ix, iy, ic, bx;
    const uint16_t *prp;
    const uint16_t *rp;
    uint16_t *wp, *cp;
    uint32_t sz;
    uint16_t b;
    uint8_t d;

    spr->w = pict->w * 2;
    spr->h = pict->h;
    spr->ncols = (spr->w + 30) >> 4;
    switch (spr->ncols) {
    case 1:
    case 2:
        switch (spr->h) {
        PLOT_HEIGHT_GENERATOR(CASE_GEN)
        default:
            panic(0x111000, spr->h);
        }
        break;

    case 3:
        if (spr->h != 6) {
            panic(0x111001, spr->h);
        }
        spr->draw_full = plot_2x6_full;
        spr->erase_full = erase_2x6_full;
        break;

    default:
        panic(0x111002, spr->ncols);
        break;
    }
    sz = 16 * spr->ncols * 12 * spr->h;
    wp = (uint16_t *)zalloc(sz);
    spr->pat[0] = wp;
    for (iy = 0; iy < spr->h; iy++) {
        for (ic = 0; ic < spr->ncols; ic++) {
            wp[ic * 6] = 0xffff; // masks
        }
        for (ix = 0; ix < spr->w; ix += 2) {
            d = pict->pat[(ix >> 1) * pict->h + iy];
            ic = ix >> 4;
            bx = ix & 0xf;
            cp = wp + ic * 6;
            if (d & 0xf0) {
                b = 1 << (15 - bx);
                cp[0] &= ~b;
                cp[2] |= (d & 0x10) ? b : 0;
                cp[3] |= (d & 0x20) ? b : 0;
                cp[4] |= (d & 0x40) ? b : 0;
                cp[5] |= (d & 0x80) ? b : 0;
            }
            if (d & 0x0f) {
                b = 1 << (15 - bx - 1);
                cp[0] &= ~b;
                cp[2] |= (d & 0x01) ? b : 0;
                cp[3] |= (d & 0x02) ? b : 0;
                cp[4] |= (d & 0x04) ? b : 0;
                cp[5] |= (d & 0x08) ? b : 0;
            }
        }
        for (ic = 0; ic < spr->ncols; ic++) {
            wp[ic * 6 + 1] = wp[ic * 6]; // dup masks
        }
        wp += spr->ncols * 6;
    }

    for (shift = 1; shift < 16; shift++) {
        spr->pat[shift] = wp;
        rp = spr->pat[shift - 1];
        for (iy = 0; iy < spr->h; iy++) {
            wp[0] = wp[1] = 0x8000 | (rp[0] >> 1);
            wp[2] = rp[2] >> 1;
            wp[3] = rp[3] >> 1;
            wp[4] = rp[4] >> 1;
            wp[5] = rp[5] >> 1;
            wp += 6; prp = rp; rp += 6;
            for (ic = 1; ic < spr->ncols; ic++) {
                wp[0] = wp[1] = ((prp[0] & 1) ? 0x8000 : 0) | (rp[0] >> 1);
                wp[2] = ((prp[2] & 1) ? 0x8000 : 0) | (rp[2] >> 1);
                wp[3] = ((prp[3] & 1) ? 0x8000 : 0) | (rp[3] >> 1);
                wp[4] = ((prp[4] & 1) ? 0x8000 : 0) | (rp[4] >> 1);
                wp[5] = ((prp[5] & 1) ? 0x8000 : 0) | (rp[5] >> 1);
                wp += 6; prp = rp; rp += 6;
            }
        }
    }
}

