/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Attract mode: Williams logo display.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "data.h"
#include "mess.h"
#include "attract.h"

#define sleep(a, b) attr_sleep((a), (b))
#define sucide()    attr_sucide()

static int logo_plot(void)
{
    uint8_t id, shift, i;
    uint32_t *dst;

    do {
        id = *gd->lgoptr++;
    } while (id == 0xff);
    if (id > 0xaa) {
        if (id == 0xfe) {
            gd->lgox = *gd->lgoptr++;
            gd->lgoy = *gd->lgoptr++;
            id = 0;
        } else {
            return 0;
        }
    }
    for (i = 0; i < 2; i++) {
        if (id & 0x80) {
            --gd->lgox;
        }
        if (id & 0x40) {
            ++gd->lgox;
        }
        if (id & 0x20) {
            --gd->lgoy;
        }
        if (id & 0x10) {
            ++gd->lgoy;
        }
        dst = (uint32_t *)SCRPTR(gd->lgox, gd->lgoy);
        shift = gd->lgox & 0xf;
        dst[0] |= col_to_planes[shift][15][0];
        dst[1] |= col_to_planes[shift][15][1];
        id <<= 4;
    }
    return -1;
}

static int logo_step(void)
{
    uint8_t n, i;

    // 60Hz code plots 18 pixels every 6 steps (3/step).
    // 50Hz code plots (4, 3, 4, 3, 4) pattern over 5 steps
    // to rate match.
    n = (gd->lgophase & 1) ? 3 : 4;
    if (++gd->lgophase == 5) {
        gd->lgophase = 0;
    }
    for (i = 0; i < n; i++) {
        if (logo_plot() == 0) {
            return 0;
        }
    }
    return 1;
}

static void pres1(void)
{
    messf(0x32 * 2, 0x58, ID_ELECTRONICS_PRESENTS);
    sleep(pres1, 5 * 5 / 6);
}

static void pres(void)
{
    mkproc(defend, AMTYPE);
    pres1();
}

static uint32_t logoimg[19 * 14];

static void draw_logoimg(void)
{
    uint32_t *swp = (uint32_t *)SCRPTR(0x68, 0x3c);
    const uint32_t *rp = logoimg;
    uint32_t *wp;
    uint8_t r, c;

    for (r = 0; r < 19; r++) {
        wp = swp;
        for (c = 0; c < 14; c++) {
            *wp++ = *rp++;
        }
        swp += 40;
    }
}

static void grab_logoimg(void)
{
    const uint32_t *srp = (uint32_t *)SCRPTR(0x68, 0x3c);
    uint32_t *wp = logoimg;
    const uint32_t *rp;
    uint8_t r, c;

    for (r = 0; r < 19; r++) {
        rp = srp;
        for (c = 0; c < 14; c++) {
            *wp++ = *rp++;
        }
        srp += 40;
    }
    if (wp != &logoimg[19 * 14]) {
        panic(0x52bc4233);
    }
}

static void logo0(void)
{
    if (gd->lgoflg == 3 || !gd->lgoimg) {
        // slow draw
        if (logo_step() == 0) {
            if (gd->lgoimg == 0) {
                grab_logoimg();
                gd->lgoimg = 1;
            }
            if (gd->lgoflg == 3) {
                gd->lgoflg = 10;
                mkproc(pres, AMTYPE);
            }
            gd->lgoptr = lgotab;
        }
        sleep(logo0, 2);
    } else {
        // fast draw
        draw_logoimg();
        sleep(logo0, 13);
    }
}

void logo(void)
{
    defnnn();
    gd->lgoflg = 3;
    gd->lgoptr = lgotab;
    gd->lgophase = 0;
    if (0) {
        sleep(defend, 1);
    } else {
        logo_step();
        sleep(logo0, 2);
    }
}

