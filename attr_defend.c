/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Attract mode: DEFENDER appearing and drawing.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "data.h"
#include "expl.h"
#include "blips.h"
#include "mess.h"
#include "attract.h"

#define sleep(a, b) attr_sleep((a), (b))
#define sucide()    attr_sucide()

static uint8_t defblk[24 * 60];    // 120x24

void creds(void)
{
    messf(0x28 << 1, 0xe5, ID_CREDITS);
    messf((0x28 + 32) << 1, 0xe5, ID_2);
    sleep(creds, 16 * 5 / 6);
}

static void cpr55(void)
{
    if (--gd->stalt != 0) {
        sleep(cpr55, 10);
    }
    haldis();
}

void plot_textimg(uint16_t x, uint8_t y, uint8_t nw,
        uint8_t h, const uint16_t *img)
{
    uint16_t *swp = SCRPTR(x, y);
    const uint16_t *rp = img;
    uint8_t r, c;
    uint16_t *wp;

    for (r = 0; r < h; r++) {
        wp = swp;
        for (c = 0; c < nw; c++) {
            *wp++ = *rp++;
            wp += 3;
        }
        swp += 80;
    }
}

void defend_resume(void)
{
    mkproc(creds, AMTYPE);
    plot_textimg(helpimg_x, 0xd0, helpimg_nw, helpimg_h, helpimg);
    gd->stalt = 50;
    cpr55();
}

static void def44(void)
{
    mkproc(cbomb, AMTYPE);
    if (gd->startup) {
        if (!gd->emain_finished) {
            plot_textimg(waitimg_x, 0xd0, waitimg_nw, waitimg_h, waitimg);
        }
        sleep(attr_wait_end, 1);
    } else {
        sleep(defend_resume, 1);
    }
}

static void wilr1(void)
{
    gd->lgoflg = 2;
    sucide();
}

static void willir(void)
{
    gd->lgoflg = 10;
    sleep(wilr1, 2);
}

static uint32_t defimg[24 * 16];

void defdraw(uint16_t x, uint8_t y)
{
    uint32_t *swp = (uint32_t *)SCRPTR(x, y);
    const uint32_t *rp = defimg;
    uint32_t *wp;
    uint8_t r, c;

    for (r = 0; r < 24; r++) {
        wp = swp;
        for (c = 0; c < 128; c += 16) {
            *wp++ = *rp++;
            *wp++ = *rp++;
        }
        swp += 40;
    }
}

static void def50b(void)
{
    if (gd->defani) {
        sleep(def50b, 1);
    }
    defdraw(0x30 << 1, 0x90);
    mkproc(willir, AMTYPE);
    sucide();
}

void mkdefimg(void)
{
    uint32_t *swp;
    uint32_t *wp;
    uint8_t r, c;
    uint16_t x;
    uint8_t d;

    if (!gd->defimg) {
        swp = defimg;
        for (r = 0; r < 24; r++) {
            wp = swp;
            swp += 16;
            x = 0;
            for (c = 0; c < 60; c++, x += 2) {
                d = defblk[c * 24 + r];
                wp[0] |= col_to_planes[x & 0xf][d >> 4][0];
                wp[1] |= col_to_planes[x & 0xf][d >> 4][1];
                wp[0] |= col_to_planes[(x + 1) & 0xf][d & 0xf][0];
                wp[1] |= col_to_planes[(x + 1) & 0xf][d & 0xf][1];
                if ((x & 0xf) == 0xe) {
                    wp += 2;
                }
            }
        }
        gd->defimg = 1;
    }
}

// "DEFENDER" whole writing routine
static void def50(void)
{
    mkdefimg();
    sleep(def50b, 1);
}

static void def33(void)
{
    mkproc(def50, AMTYPE);
    sleep(def44, 0x28 * 5 / 6);
}

typedef struct {
    uint16_t id;
    fragpat_t fragpat;
} dfragpat_t;

#define NDFRAGPATS 38
static dfragpat_t dfragpats[NDFRAGPATS];
static uint8_t ndfragpats;

static fragpat_t *mkdfragpat(uint8_t d0, uint8_t d1)
{
    uint16_t id = (d0 << 8) | d1;
    uint8_t i;

    for (i = 0; i < ndfragpats; i++) {
        if (dfragpats[i].id == id) {
            break;
        }
    }
    if (i == ndfragpats) {
        if (i == NDFRAGPATS) {
            panic(0xefd54baa, id);
        }
        dfragpats[i].id = id;
        mkfrag(&dfragpats[i].fragpat, d0, d1);
        ++ndfragpats;
    }
    return &dfragpats[i].fragpat;
}

typedef struct {
    const fragpat_t *fragpat;
    uint32_t eramask;
    uint32_t *eradst;
} dfrag_t;

typedef struct {
    uint8_t cx;
    uint8_t cy;
    int32_t rsize;
    dfrag_t frags[24];
} dexpl_t;

static dexpl_t dexpls[15];

// 0 1 2 3
// O O|O O 0
// O O|O O 1
// O O|O O 2
// ---+---
// O O|O O 3
// O O|O O 4
// O O|O O 5
static void dexpl_step(dexpl_t *ep)
{
    int32_t step_x, step_y;
    int32_t cx = ep->cx << 9;
    int32_t cy = ep->cy << 8;
    int32_t ox, oy, x, y;
    const fragpat_t *fp;
    int16_t px, py;
    uint16_t shift;
    uint32_t *dst;
    uint8_t c, r;
    dfrag_t *dp;

    ep->rsize -= 0xc00;
    if (ep->rsize <= 0) {
        dp = ep->frags;
        for (c = 0; c < 24; c++, dp++) {
            if (dp->eradst != NULL) {
                dp->eradst[0] &= dp->eramask;
                dp->eradst[1] &= dp->eramask;
                dp->eradst[40] &= dp->eramask;
                dp->eradst[41] &= dp->eramask;
            }
        }
        return;
    }

    step_x = -ep->rsize >> 2;
    step_y = -ep->rsize >> 2;
    ox = cx - step_x - (step_x >> 1);          // cx - 1.5 * step_x
    oy = cy - step_y - step_y - step_y;        // cy - 2 * step_y
    dp = ep->frags;
    x = ox;
    for (c = 0; c < 4; c++, x += step_x) {
        y = oy;
        for (r = 0; r < 6; r++, dp++, y += step_y) {
            if (dp->eradst != NULL) {
                dp->eradst[0] &= dp->eramask;
                dp->eradst[1] &= dp->eramask;
                dp->eradst[40] &= dp->eramask;
                dp->eradst[41] &= dp->eramask;
            }
            px = x >> 8;
            py = y >> 8;
            if (px >= 16 && px < 304 &&
                py >= YMIN && py < YMAX) {
                dst = (uint32_t *)SCRPTR(px, py);
                shift = (px & 0xf) >> 1;
                fp = dp->fragpat;
                dst[0] |= fp->pat[shift][0];
                dst[1] |= fp->pat[shift][1];
                dst[40] |= fp->pat[shift][2];
                dst[41] |= fp->pat[shift][3];
                dp->eradst = dst;
                dp->eramask = blipmask[shift];
            } else {
                dp->eradst = NULL;
            }
        }
    }
}

static void defani(void)
{
    dexpl_t *dp;
    uint16_t i;

    //eorcolor(0x700);
    dp = dexpls;
    for (i = 0; i < 15; i++, dp++) {
        dexpl_step(dp);
    }
    //eorcolor(0x700);
    if (dexpls[0].rsize <= 0) {
        gd->defani = 0;
        sucide();
    }
    sleep(defani, 1);
}

// Appear as 15 8x12 pieces, each piece having 24 frags
static void defens(void)
{
    uint8_t d0, d1, c, r;
    const uint8_t *fsp;
    const uint8_t *sp;
    dexpl_t *ep;
    dfrag_t *dp;
    int8_t i;

    sp = defblk;
    ep = dexpls;
    for (i = 0; i < 15; i++, ep++) {
        ep->rsize = 0x20000;
        ep->cx = 0x30 + (i << 2) + 2;
        ep->cy = 0x98;
        dp = ep->frags;
        sp += 4 * 24;
        fsp = sp;
        r = 0;
        c = 0;
        for (c = 0; c < 4; c++) {
            for (r = 0; r < 6; r++, dp++) {
                d0 = *fsp++;
                d1 = *fsp++;
                dp->fragpat = mkdfragpat(d0, d1);
                dp->eradst = NULL;
            }
        }
    }

    // animate appear
    gd->defani = 1;
    mkproc(defani, AMTYPE);

    // start appear
    sleep(def33, 0x2e * 5 / 6);
}

void defend(void)
{
    sleep(defens, 0x30 * 5 / 6);
}

// "defender" data expander
static void defx2(uint8_t a)
{
    if ((a & 0x0c) == 0) {
        // extend
        gd->len = (a + gd->len) << 2;
        return;
    }
    // direct
    gd->len += a & 0x3;
    gd->dcolor = dcolrs[((a & 0xc) >> 2) - 1];
    if (gd->defwptr >= &defblk[24*60]) {
        gd->defwptr += 1 - 24*60;   // next row
    }
    a = gd->pix;
    if (a != 0) {
        *gd->defwptr = (*gd->defwptr & 0xf0) | (gd->dcolor & 0x0f);
        a = gd->dcolor;
        gd->defwptr += 24;      // next col
        --gd->len;
        if (gd->len < 0) {
            gd->pix = 0;
            gd->len = 0;
            return;
        }
    } else {
        gd->pix ^= 0xff;
        a = gd->dcolor;
    }
    do {
        *gd->defwptr = a;
        --gd->len;
        if (gd->len < 0) {
            gd->len = 0;
            return;
        }
        gd->defwptr += 24;      // next col
        --gd->len;
    } while (gd->len >= 0);
    gd->pix = 0;
    gd->len = 0;
}

void defnnn(void)
{
    const uint8_t *y;
    uint16_t n;

    if (gd->defwptr != NULL) {
        return;
    }
    gd->defwptr = defblk;
    y = defdat;
    gd->pix = 0;
    gd->len = 0;
    n = defdatlen;
    while (n--) {
        defx2(*y >> 4);
        defx2(*y++ & 0xf);
    }
}

