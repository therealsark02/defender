/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Color management and cycling.
 */

#include "gd.h"
#include "attract.h"
#include "sprites.h"
#include "color.h"

#define sleep(a, b) attr_sleep((a), (b))

// map from a Defender 8-bit color to an ST 12-bit color.
static uint16_t col8to12[256];

// 4-plane color of a single pixel in all shift/colors.
// [shift][col][idx]
// idx=0 is planes 0&1, idx=1 is planes 2&3.
uint32_t col_to_planes[16][16][2];

uint8_t pcram[16];

static void build_col8to12(void)
{
    uint16_t c, r, g, b;

    for (c = 0; c < 256; c++) {
        b = (c >> 6) & 0x3;         // bb------
        g = (c >> 3) & 0x7;         // --ggg---
        r = (c >> 0) & 0x7;         // -----rrr
        b = (b << 1) | (b > 1);
        col8to12[c] = (r << 8) | (g << 4)| b;
    }
}

static void build_col_to_planes(void)
{
    uint8_t col, shift;

    for (col = 0; col < 16; col++) {
        col_to_planes[0][col][0] =
                ((col & 0x1) ? 0x80000000 : 0) |
                ((col & 0x2) ? 0x00008000 : 0);
        col_to_planes[0][col][1] =
                ((col & 0x4) ? 0x80000000 : 0) |
                ((col & 0x8) ? 0x00008000 : 0);
        for (shift = 1; shift < 16; shift++) {
            col_to_planes[shift][col][0] = col_to_planes[shift - 1][col][0] >> 1;
            col_to_planes[shift][col][1] = col_to_planes[shift - 1][col][1] >> 1;
        }
    }
}

void crinit_maps(void)
{
    build_col8to12();
    build_col_to_planes();
}

void colr_apply(void)
{
    uint16_t *hwpal = (uint16_t *)0xffff8240;
    uint8_t i;

    for (i = gd->want_timing ? 1 : 0; i < 16; i++) {
        hwpal[i] = col8to12[pcram[i]];
    }
}

void crinit(void)
{
    static const uint8_t crtab[16] = {
        0x00,   // 0 space
        0x00,   // 1 laser (special)
        0x07,   // 2 red
        0x28,   // 3 green
        0x2f,   // 4 yellow
        0x81,   // 5 blue
        0xa4,   // 6 gray
        0xc7,   // 7 purple (brown in original)
        0x15,   // 8 brown  (purple in original)
        0xff,   // 9 white
        0x00,   // a bomb cycler
        0x00,   // b monochrome
        0x00,   // c cycler
        0x00,   // d tie1
        0x00,   // e tie2
        0x00,   // f tie3
    };
    int i;

    // initialize pcram
    for (i = 0; i < 16; i++) {
        pcram[i] = crtab[i];
    }
    if (gd->cheats) {
        pcram[0] = 0x01;
    }
    colr_apply();
}

static void tiecl(void)
{
    // tie color table
    static const uint8_t tctab[3][3] = {
        { 0x81, 0x81, 0x2f },
        { 0x81, 0x2f, 0x07 },
        { 0x2f, 0x81, 0x07 }
    };
    proc_t *p = gd->crproc;
    uint8_t idx = p->pd[0];

    pcram[13] = tctab[idx][0];
    pcram[14] = tctab[idx][1];
    pcram[15] = tctab[idx][2];
    if (++idx == 3) {
        idx = 0;
    }
    p->pd[0] = idx;
    sleep(tiecl, 6);
}

void tiecol(void)
{
    proc_t *p = gd->crproc;

    p->pd[0] = 0;       // initialize table index
    sleep(tiecl, 5);
}

/*
 * color table
 */
const uint8_t coltab[] = {
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x37, 0x2f, 0x27,
    0x1f, 0x17, 0x47, 0x47, 0x87, 0x87, 0xc7, 0xc7, 0xc6, 0xc5, 0xcc,
    0xcb, 0xca, 0xda, 0xe8, 0xf8, 0xf9, 0xfa, 0xfb, 0xfd, 0xff, 0xbf,
    0x3f, 0x3e, 0x3c, 0
};

static void cbmb1(void)
{
    uint8_t col;

    col = coltab[gd->seed & 0x1f];
    pcram[10] = col;
    pcram[12] = col;
    gd->bax = (gd->bax == &sprtab[P_BMBP1]) ? &sprtab[P_BMBP2] :
                &sprtab[P_BMBP1];
    sleep(cbomb, 6);
}

void cbomb(void)
{
    pcram[10] = 0xff;
    pcram[12] = 0;
    sleep(cbmb1, 3);
}

static void colrlp(void)
{
    proc_t *p = gd->crproc;
    uint8_t idx;
    uint8_t col;

    p = gd->crproc;
    idx = p->pd[0];
    col = coltab[idx++];
    if (col == 0) {
        idx = 0;
        col = coltab[idx++];
    }
    pcram[1] = col;
    p->pd[0] = idx;
    sleep(colrlp, 2);
}

void colr(void)
{
    proc_t *p = gd->crproc;

    p->pd[0] = 0;
    colrlp();
}
