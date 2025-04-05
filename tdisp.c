/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Top of screen display.
 */

#include "gd.h"
#include "screen.h"
#include "scanner.h"
#include "plstrt.h"
#include "sprites.h"
#include "sound.h"
#include "tdisp.h"

static const uint32_t lmasktab[16] = {
    0x00000000, 0x80008000, 0xc000c000, 0xe000e000,
    0xf000f000, 0xf800f800, 0xfc00fc00, 0xfe00fe00,
    0xff00ff00, 0xff80ff80, 0xffc0ffc0, 0xffe0ffe0,
    0xfff0fff0, 0xfff8fff8, 0xfffcfffc, 0xfffefffe,
};

static const uint32_t rmasktab[16] = {
    0x00000000, 0x00010001, 0x00030003, 0x00070007,
    0x000f000f, 0x001f001f, 0x003f003f, 0x007f007f,
    0x00ff00ff, 0x01ff01ff, 0x03ff03ff, 0x07ff07ff,
    0x0fff0fff, 0x1fff1fff, 0x3fff3fff, 0x7fff7fff,
};

void blkclr(uint16_t x, uint8_t y, uint16_t w, uint8_t h)
{
    uint32_t *dst = (uint32_t *)SCRPTR(x, y);
    uint8_t lshift = x & 0xf;
    uint8_t rshift = (16 - (x + w)) & 0xf;
    uint32_t lmask = lmasktab[lshift];
    uint32_t rmask = rmasktab[rshift];
    uint16_t sgx = x & 0xfff0;
    uint16_t egx = (x + w - 1) & 0xfff0;
    uint32_t cmask;
    uint32_t *wdst;
    uint8_t i;

    if (sgx == egx) {
        // single word
        cmask = lmask | rmask;
        if (cmask == 0) {
            while (h--) {
                dst[0] = 0;
                dst[1] = 0;
                dst += 40;
            }
        } else {
            while (h--) {
                dst[0] &= cmask;
                dst[1] &= cmask;
                dst += 40;
            }
        }
    } else {
        // multi word
        // sgx < egx
        wdst = dst;
        if (lmask != 0) {
            for (i = 0; i < h; i++) {
                wdst[0] &= lmask;
                wdst[1] &= lmask;
                wdst += 40;
            }
        } else {
            for (i = 0; i < h; i++) {
                wdst[0] = 0;
                wdst[1] = 0;
                wdst += 40;
            }
        }
        dst += 2;
        sgx += 16;
        while (sgx != egx) {
            wdst = dst;
            for (i = 0; i < h; i++) {
                wdst[0] = 0;
                wdst[1] = 0;
                wdst += 40;
            }
            dst += 2;
            sgx += 16;
        }
        wdst = dst;
        if (rmask != 0) {
            for (i = 0; i < h; i++) {
                wdst[0] &= rmask;
                wdst[1] &= rmask;
                wdst += 40;
            }
        } else {
            for (i = 0; i < h; i++) {
                wdst[0] = 0;
                wdst[1] = 0;
                wdst += 40;
            }
        }
    }
}

void border(void)
{
    uint16_t *wp;
    int i;

    wp = SCRPTR(16, SCANER_Y + 0x20);
    for (i = 0; i < SCREEN_COLS - 2; i++) {
        wp[0] = 0xffff;
        wp[2] = 0xffff;
        wp[80] = 0xffff;
        wp[82] = 0xffff;
        wp += 4;
    }
    wp = (uint16_t *)SCRPTR(SCANER_X, SCANER_Y - 1);
    for (i = 0; i < 8; i++) {
        wp[0] = 0xffff;
        wp[2] = 0xffff;
        wp += 4;
    }
    wp = (uint16_t *)SCRPTR(SCANER_X - 16, SCANER_Y - 1);
    for (i = 0; i < 0x21; i++) {
        wp[0] = 0x0003;
        wp[2] = 0x0003;
        wp[36] = 0xc000;
        wp[38] = 0xc000;
        wp += 80;
    }
    wp = (uint16_t *)SCRPTR(0x4c*2, SCANER_Y - 1);
    wp[0]=0xffff; wp[4] = 0xffff;
    wp[2]=0xff00; wp[6] = 0x00ff;
    wp[3]=0x00ff; wp[7] = 0xff00;
    wp += 80;
    wp[0]=0x00ff; wp[4] = 0xff00;
    wp[2]=0x0000; wp[6] = 0x0000;
    wp[3]=0x00ff; wp[7] = 0xff00;
    wp = (uint16_t *)SCRPTR(0x4c*2, SCANH + 0x20);
    wp[0]=0xffff; wp[4] = 0xffff;
    wp[2]=0xff00; wp[6] = 0x00ff;
    wp[3]=0x00ff; wp[7] = 0xff00;
    wp += 80;
    wp[0]=0xffff; wp[4] = 0xffff;
    wp[2]=0xff00; wp[6] = 0x00ff;
    wp[3]=0x00ff; wp[7] = 0xff00;
}

void coff(uint16_t x, uint8_t y, const sprpat_t *spr)
{
    uint8_t w = spr->w;
    uint8_t shift = x & 0xf;
    const uint16_t *src;
    uint16_t *dst;

    src = spr->pat[shift];
    dst = SCRPTR(x, y);

    // fully visible
    if ((16 - shift) < w) {
        // spans 2 cols
        spr->erase_full(dst, src);
    } else {
        // spans 1 col
        spr->erase_left(dst, src);
    }
}

void cwrit(uint16_t x, uint8_t y, const sprpat_t *spr)
{
    uint8_t w = spr->w;
    uint8_t shift = x & 0xf;
    const uint16_t *src;
    uint16_t *dst;

    src = spr->pat[shift];
    dst = SCRPTR(x, y);

    // fully visible
    if ((16 - shift) < w) {
        // spans 2 cols
        spr->draw_full(dst, src);
    } else {
        // spans 1 col
        spr->draw_left(dst, src);
    }
}

static void ldsp(uint16_t x, uint8_t y, uint8_t n)
{
    if (n > 5) {
        n = 5;
    }
    blkclr(x, y, 64, 6);
    while (n--) {
        cwrit(x, y, &sprtab[P_PLAMIN]);
        x += 12;
    }
}

static void ldisp(void)
{
    ldsp(30, 20, pldata[0].plas);
    if (gd->plrcnt == 2) {
        ldsp(226, 20, pldata[1].plas);
    }
}

static void sbdsp(uint16_t x, uint8_t y, uint8_t n)
{
    if (n > 3) {
        n = 3;
    }
    blkclr(x, y, 6, 11);
    while (n--) {
        cwrit(x, y, &sprtab[P_SBPIC]);
        y += 4;
    }
}

void sbdisp(void)
{
    sbdsp(82, 27, pldata[0].psbc);
    if (gd->plrcnt == 2) {
        sbdsp(278, 27, pldata[1].psbc);
    }
}

void scrtr0(uint8_t pid)
{
    const uint8_t *scp = (uint8_t *)&pldata[pid - 1].pscor + 1;
    uint16_t x = (pid == 1) ? 30 : 226;
    uint8_t i, d, z;
    uint8_t y = 28;

    z = 0;
    for (i = 0; i < 6; i++) {
        d = *scp++;
        if (!(i & 1)) {
            d >>= 4;
            --scp;
        }
        d &= 0xf;
        coff(x, y, &sprchrtab[C_BLANK3X8]);
        if (d != 0 || z) {
            cwrit(x, y, &sprchrtab[C_NUMBR0 + d]);
            z += (d != 0);
        }
        z += (i > 2);
        x += 8;
    }
}

void scrtrn(void)
{
    scrtr0(gd->curplr);
}

void add_bcd(uint32_t *a, uint32_t b);

asm (
    "add_bcd:\n"
    "       lea     12(%sp),%a0\n"
    "       move.l  4(%sp),%a1\n"
    "       addq    #4,%a1\n"
    "       andi    #0,%ccr\n"
    "       abcd    -(%a0),-(%a1)\n"
    "       abcd    -(%a0),-(%a1)\n"
    "       abcd    -(%a0),-(%a1)\n"
    "       abcd    -(%a0),-(%a1)\n"
    "       rts");

//A=0-7EXP,B=0-99
void score(uint16_t pts)
{
    pldata_t *pyr = plindx();
    uint8_t exp, mant;
    uint32_t scr;

    gd->scrflg = (gd->scrflg << 1) | 1;
    exp = pts >> 8;
    mant = pts & 0xff;
    scr = mant << (4 * exp);
    add_bcd(&pyr->pscor, scr);

    // check replay
    if (gd->repla != 0 && (pyr->pscor & 0xffffff) >= (pyr->prpla & 0xffffff)) {
        add_bcd(&pyr->prpla, gd->repla << 8);
        ++pyr->plas;
        ++pyr->psbc;
        ldisp();
        sbdisp();
        sndld(rpsnd);
    }
    scrtrn();
}

static void flp2(void)
{
    scrtrn();
    sleep(flpup, 12);
}

void flpup(void)
{
    uint16_t x = (gd->curplr == 1) ? 30 : 226;
    uint8_t y = 28;

    if (gd->scrflg) {
        sucide();
    }
    blkclr(x, y, 48, 8);
    sleep(flp2, 8);
}

void tdisp(void)
{
    uint8_t i;

    blkclr(SCANER_X, SCANER_Y, 0x80, 0x20);
    border();
    ldisp();
    sbdisp();
    for (i = 1; i <= gd->plrcnt; i++) {
        scrtr0(i);
    }
}

