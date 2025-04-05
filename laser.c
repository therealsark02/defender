/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Frikin' lasers, man (the player's primary weapon).
 */

#include "gd.h"
#include "screen.h"
#include "sprites.h"
#include "collide.h"
#include "sound.h"
#include "irq.h"
#include "rand.h"

// tip: 12 pixels
// 8 shift patterns, 4 words (32px over 2 planes)
static uint16_t lasertipr[8][4];
static uint16_t lasertipl[8][4];

// body: 6px from 16 fiss patterns
// 8 shift patterns, 4 words (32px over 2 planes)
static uint16_t laserbody[16][8][4];

// tail: erase 2px or 4px
// 8 shift patterns, 2 words (32px over 1 common plane mask)
static uint16_t lasertailr[2][8][2];
static uint16_t lasertaill[2][8][2];

// tailzap: erase 16,14,12..2px
// 8 patterns, 1 word (16px over 1 common plane mask)
static uint16_t lasertailzapr[8];
static uint16_t lasertailzapl[8];

// laser collision
static int lcol(uint8_t x, uint8_t y)
{
    return colide(x << 1, y, &sprtab[P_LASP1]);
}

// every frame:
//   - @pd[0]: plot 10 px color 1, 2 px color 9
//   - @pd[1]: plot 6 px color 0&1 from fiss table
//   - @pd[2]: erase 2 px, or 4 every 5th frame
// laser advances 10px/frame NTSC, so 12px/frame for PAL
// laser erases 2px/frame NTSC, so 2.4px/frame for PAL
//  2.4     2   5 -> 4
//  4.8     4   4 -> 3
//  7.2     6   3 -> 2
//  9.6     8   2 -> 1
// 12.0    10   1 -> 0
//   - @pd[3]: cnt
static void lasr0(void)
{
    proc_t *p = gd->crproc;
    uint8_t pat;
    laser_t *lp;

    if (!(gd->status & ST_ATTRACT)) {       // player controls inactive?
        // no
        lp = &p->laser.pd[0];              // tip
        if (lp->dst < p->laser.eol) {      // dst < end of line?
            // no, plot it...
            // erase the plane 3 2px tip
            lp->dst[3] &= lasertailr[0][lp->shift][0];
            // plot the rest
            lp->dst[0] |= lasertipr[lp->shift][0];
            lp->dst[3] |= lasertipr[lp->shift][1];
            if (lp->dst < p->laser.eol - 4) {
                lp->dst[4] |= lasertipr[lp->shift][2];
                lp->dst[7] |= lasertipr[lp->shift][3];
            }
            p->laser.x += 5;                // advance x 10px
            lp->shift += 5;
            if (lp->shift >= 8) {
                lp->shift &= 0x7;
                lp->dst += 4;
            }

            lp = &p->laser.pd[1];           // body
            pat = gd->lasrpat;
            gd->lasrpat = (gd->lasrpat + 1) & 0xf;
            lp->dst[0] &= laserbody[pat][lp->shift][0];
            lp->dst[3] &= laserbody[pat][lp->shift][1];
            lp->dst[4] &= laserbody[pat][lp->shift][2];
            lp->dst[7] &= laserbody[pat][lp->shift][3];
            lp->shift += 3;                 // advance 6px
            if (lp->shift >= 8) {
                lp->shift &= 0x7;
                lp->dst += 4;
            }

            lp = &p->laser.pd[2];            // tail
            if (--p->laser.tailcnt == 0) {
                pat = 1;    // 4px
            } else {
                pat = 0;    // 2px
            }
            lp->dst[0] &= lasertailr[pat][lp->shift][0];
            lp->dst[3] &= lasertailr[pat][lp->shift][0];
            lp->dst[4] &= lasertailr[pat][lp->shift][1];
            lp->dst[7] &= lasertailr[pat][lp->shift][1];
            lp->shift += pat ? 2 : 1;       // advance 2px or 4px
            if (lp->shift >= 8) {
                lp->shift &= 0x7;
                lp->dst += 4;
            }

            if (lcol(p->laser.x - 6, p->laser.y) == 0) {    // collide?
                // no, keep going
                sleep(lasr0, 1);
            }
        }
    }
    lp = &p->laser.pd[2];            // tail
    if (lp->dst < p->laser.eol) {
        if (lp->shift != 0) {
            lp->dst[0] &= lasertailzapr[lp->shift];
            lp->dst[3] &= lasertailzapr[lp->shift];
            lp->dst += 4;
        }
        while (lp->dst < p->laser.eol) {
            lp->dst[0] = 0;
            lp->dst[3] = 0;
            lp->dst += 4;
        }
    }
    --gd->lflg;                 // one fewer laser in flight
    sucide();
}

// attract mode version with no collision
static void attr_lasr0(void)
{
    proc_t *p = gd->crproc;
    uint8_t pat;
    laser_t *lp;

    lp = &p->laser.pd[0];              // tip
    if (lp->dst < p->laser.eol) {      // dst < end of line?
        // no, plot it...
        // erase the plane 3 2px tip
        lp->dst[3] &= lasertailr[0][lp->shift][0];
        // plot the rest
        lp->dst[0] |= lasertipr[lp->shift][0];
        lp->dst[3] |= lasertipr[lp->shift][1];
        if (lp->dst < p->laser.eol - 4) {
            lp->dst[4] |= lasertipr[lp->shift][2];
            lp->dst[7] |= lasertipr[lp->shift][3];
        }
        p->laser.x += 5;                // advance x 10px
        lp->shift += 5;
        if (lp->shift >= 8) {
            lp->shift &= 0x7;
            lp->dst += 4;
        }

        lp = &p->laser.pd[1];           // body
        pat = gd->lasrpat;
        gd->lasrpat = (gd->lasrpat + 1) & 0xf;
        lp->dst[0] &= laserbody[pat][lp->shift][0];
        lp->dst[3] &= laserbody[pat][lp->shift][1];
        lp->dst[4] &= laserbody[pat][lp->shift][2];
        lp->dst[7] &= laserbody[pat][lp->shift][3];
        lp->shift += 3;                 // advance 6px
        if (lp->shift >= 8) {
            lp->shift &= 0x7;
            lp->dst += 4;
        }

        lp = &p->laser.pd[2];            // tail
        if (--p->laser.tailcnt == 0) {
            pat = 1;    // 4px
        } else {
            pat = 0;    // 2px
        }
        lp->dst[0] &= lasertailr[pat][lp->shift][0];
        lp->dst[3] &= lasertailr[pat][lp->shift][0];
        lp->dst[4] &= lasertailr[pat][lp->shift][1];
        lp->dst[7] &= lasertailr[pat][lp->shift][1];
        lp->shift += pat ? 2 : 1;       // advance 2px or 4px
        if (lp->shift >= 8) {
            lp->shift &= 0x7;
            lp->dst += 4;
        }
        sleep(attr_lasr0, 1);
    }
    lp = &p->laser.pd[2];            // tail
    if (lp->dst < p->laser.eol) {
        if (lp->shift != 0) {
            lp->dst[0] &= lasertailzapr[lp->shift];
            lp->dst[3] &= lasertailzapr[lp->shift];
            lp->dst += 4;
        }
        while (lp->dst < p->laser.eol) {
            lp->dst[0] = 0;
            lp->dst[3] = 0;
            lp->dst += 4;
        }
    }
    sucide();
}

static void lasl0(void)
{
    proc_t *p = gd->crproc;
    uint8_t pat;
    laser_t *lp;

    if (!(gd->status & ST_ATTRACT)) {       // player controls inactive?
        // no
        lp = &p->laser.pd[0];              // tip
        if (lp->dst >= p->laser.eol) { // dst still on screen?
            // yes, plot it...
            // erase the plane 3 2px tip
            lp->dst[3] &= lasertaill[0][lp->shift][1];
            lp->dst[0] |= lasertipl[lp->shift][2];
            lp->dst[3] |= lasertipl[lp->shift][3];
            if (lp->dst >= p->laser.eol + 4) {
                lp->dst[-4] |= lasertipl[lp->shift][0];
                lp->dst[-1] |= lasertipl[lp->shift][1];
            }
            p->laser.x -= 5;                // advance x 10px
            lp->shift -= 5;
            if (lp->shift < 0) {
                lp->shift &= 0x7;
                lp->dst -= 4;
            }

            lp = &p->laser.pd[1];           // body
            pat = gd->lasrpat;
            gd->lasrpat = (gd->lasrpat + 1) & 0xf;
            lp->dst[0] &= laserbody[pat][lp->shift][0];
            lp->dst[3] &= laserbody[pat][lp->shift][1];
            lp->dst[4] &= laserbody[pat][lp->shift][2];
            lp->dst[7] &= laserbody[pat][lp->shift][3];
            lp->shift -= 3;                 // advance 6px
            if (lp->shift < 0) {
                lp->shift &= 0x7;
                lp->dst -= 4;
            }

            lp = &p->laser.pd[2];            // tail
            if (--p->laser.tailcnt == 0) {
                pat = 1;    // 4px
            } else {
                pat = 0;    // 2px
            }
            lp->dst[0] &= lasertaill[pat][lp->shift][1];
            lp->dst[3] &= lasertaill[pat][lp->shift][1];
            lp->dst[-4] &= lasertaill[pat][lp->shift][0];
            lp->dst[-1] &= lasertaill[pat][lp->shift][0];
            lp->shift -= pat ? 2 : 1;       // advance 2px or 4px
            if (lp->shift < 0) {
                lp->shift &= 0x7;
                lp->dst -= 4;
            }
            if (lcol(p->laser.x, p->laser.y) == 0) {    // collide?
                // no, keep going
                sleep(lasl0, 1);
            }
        }
    }
    lp = &p->laser.pd[2];            // tail
    if (lp->dst >= p->laser.eol) {
        if (lp->shift != 0) {
            lp->dst[0] &= lasertailzapl[lp->shift];
            lp->dst[3] &= lasertailzapl[lp->shift];
            lp->dst -= 4;
        }
        while (lp->dst >= p->laser.eol) {
            lp->dst[0] = 0;
            lp->dst[3] = 0;
            lp->dst -= 4;
        }
    }
    --gd->lflg;                 // one fewer laser in flight
    sucide();
}

static uint8_t fistab[32];

static void fiss(void)
{
    uint8_t *wp = fistab;
    uint8_t a, c;

    do {
        a = rand();
        c = 0;
        if (a & 0x1) {
            c |= 0x01;
        }
        if (a & 0x2) {
            c |= 0x10;
        }
        *wp++ = c;
    } while (wp != &fistab[32]);
}

void laser_init(void)
{
    const uint8_t *fisx = fistab;
    uint16_t w0, w1, w2, w3;
    uint16_t i, j;
    uint16_t *wp;
    uint8_t c;

    // tip - 10px color 1 + 2px color 9
    wp = &lasertipr[0][0];
    wp[0] = w0 = 0xfff0;
    wp[1] = w1 = 0x0030;
    wp[2] = w2 = 0;
    wp[3] = w3 = 0;
    wp += 4;
    for (i = 0; i < 7; i++) {
        w3 = (w1 << 14) | (w3 >> 2);
        w2 = (w0 << 14) | (w2 >> 2);
        w1 >>= 2;
        w0 >>= 2;
        wp[0] = w0;
        wp[1] = w1;
        wp[2] = w2;
        wp[3] = w3;
        wp += 4;
    }
    wp = &lasertipl[0][0];
    wp[0] = w0 = 0x03ff;
    wp[1] = w1 = 0x0300;
    wp[2] = w2 = 0xc000;
    wp[3] = w3 = 0x0000;
    wp += 4;
    for (i = 0; i < 7; i++) {
        w3 = (w1 << 14) | (w3 >> 2);
        w2 = (w0 << 14) | (w2 >> 2);
        w1 >>= 2;
        w0 >>= 2;
        wp[0] = w0;
        wp[1] = w1;
        wp[2] = w2;
        wp[3] = w3;
        wp += 4;
    }

    // body - 6px color 0 or 1 from fistab
    fiss();
    wp = &laserbody[0][0][0];
    for (i = 0; i < 16; i++) {
        if (fisx > &fistab[29]) {
            fisx = fistab;
        }
        w0 = 0xffff;
        w1 = 0x03ff;
        w2 = 0xffff;
        w3 = 0xffff;
        c = *fisx++;
        if ((c >> 4) == 0)  w0 &= ~0x8000;
        if ((c & 0xf) == 0) w0 &= ~0x4000;
        c = *fisx++;
        if ((c >> 4) == 0)  w0 &= ~0x2000;
        if ((c & 0xf) == 0) w0 &= ~0x1000;
        c = *fisx++;
        if ((c >> 4) == 0)  w0 &= ~0x0800;
        if ((c & 0xf) == 0) w0 &= ~0x0400;
        wp[0] = w0;
        wp[1] = w1;
        wp[2] = w2;
        wp[3] = w3;
        wp += 4;
        for (j = 0; j < 7; j++) {
            w3 = (w1 << 14) | (w3 >> 2);
            w2 = (w0 << 14) | (w2 >> 2);
            w1 = 0xc000 | (w1 >> 2);
            w0 = 0xc000 | (w0 >> 2);
            wp[0] = w0;
            wp[1] = w1;
            wp[2] = w2;
            wp[3] = w3;
            wp += 4;
        }
    }

    // tail - 2px or 4px erased
    wp = &lasertailr[0][0][0];
    for (i = 0; i < 2; i++) {
        w0 = (i == 0) ? 0x3fff : 0x0fff;
        w1 = 0xffff;
        wp[0] = w0;
        wp[1] = w1;
        wp += 2;
        for (j = 0; j < 7; j++) {
            w1 = (w0 << 14) | (w1 >> 2);
            w0 = 0xc000 | (w0 >> 2);
            wp[0] = w0;
            wp[1] = w1;
            wp += 2;
        }
    }
    wp = &lasertaill[0][0][0];
    for (i = 0; i < 2; i++) {
        w0 = (i == 0) ? 0xffff : 0xfffc;
        w1 = (i == 0) ? 0x3fff : 0x3fff;
        wp[0] = w0;
        wp[1] = w1;
        wp += 2;
        for (j = 0; j < 7; j++) {
            w1 = (w0 << 14) | (w1 >> 2);
            w0 = 0xc000 | (w0 >> 2);
            wp[0] = w0;
            wp[1] = w1;
            wp += 2;
        }
    }

    // tailzp - rest of word at various offsets
    wp = &lasertailzapr[0];
    for (i = 0; i < 8; i++) {
        *wp++ = ~(0xffff >> (i * 2));
    }
    wp = &lasertailzapl[0];
    for (i = 0; i < 8; i++) {
        *wp++ = (1 << (7 - i)) - 1;
    }
}

static void lasr(void)
{
    proc_t *p = gd->crproc;

    p->laser.x = gd->nplaxc + 7;
    p->laser.y = gd->nplayc + 4;

    p->laser.eol = SCRPTR(304, p->laser.y);
    p->laser.tailcnt = 5;

    p->laser.pd[0].dst = SCRPTR(p->laser.x * 2, p->laser.y);
    p->laser.pd[0].shift = (p->laser.x & 0xf) >> 1;
    p->laser.pd[1] = p->laser.pd[0];
    p->laser.pd[2] = p->laser.pd[0];

    lasr0();
}

// attract mode version
void attr_lasr(void)
{
    proc_t *p = gd->crproc;

    p->laser.x = gd->nplaxc + 7;
    p->laser.y = gd->nplayc + 4;

    p->laser.eol = SCRPTR(gd->lasreol << 1, p->laser.y);
    p->laser.tailcnt = 5;

    p->laser.pd[0].dst = SCRPTR(p->laser.x * 2, p->laser.y);
    p->laser.pd[0].shift = (p->laser.x & 0xf) >> 1;
    p->laser.pd[1] = p->laser.pd[0];
    p->laser.pd[2] = p->laser.pd[0];

    attr_lasr0();
}

static void lasl(void)
{
    proc_t *p = gd->crproc;

    p->laser.x = gd->nplaxc;
    p->laser.y = gd->nplayc + 4;

    p->laser.eol = SCRPTR(16, p->laser.y);
    p->laser.tailcnt = 5;

    p->laser.pd[0].dst = SCRPTR(p->laser.x * 2, p->laser.y);
    p->laser.pd[0].shift = p->laser.x & 7;
    p->laser.pd[1].dst = SCRPTR((p->laser.x - 10) * 2, p->laser.y);
    p->laser.pd[1].shift = (p->laser.x - 10) & 7;
    p->laser.pd[2] = p->laser.pd[0];

    lasl0();
}

void lfire(void)
{
    if (gd->lflg >= 4) {
        sucide();
    }
    ++gd->lflg;
    sndld(lassnd);
    if (gd->nplad >= 0) {
        lasr();
    } else {
        lasl();
    }
}

