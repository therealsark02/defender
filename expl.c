/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Explosions (get bigger) and Appears (get smaller).
 */

#include "gd.h"
#include "sprites.h"
#include "screen.h"
#include "obj.h"
#include "expl.h"

static const uint8_t fragtopic[EX__NUM] = {
    0,
    P_SCZP1,        // 5x4 = 20 frags
    P_ASXP1,        // 4x4 = 16 frags
    P_SWXP1,        // 4x4 = 16 frags
    P_PRBP1,        // 4x4 = 16 frags
    P_ASTP2,        // 2x4 =  8 frags
    P_ASTP4,        // 2x4 =  8 frags
    P_TIEP1,        // 4x4 = 16 frags
    P_BXPIC,        // 4x4 = 16 frags
    P_SWPIC1,       // 3x2 =  6 frags
    P_LNDP1,        // 5x4 = 20 frags
    P_UFOP1,        // 6x2 = 12 frags
    P_TEREX,        // 8x3 = 24 frags
    P_PLAPIC,       // 8x3 = 24 frags
    P_PLBPIC,       // 8x3 = 24 frags
};

static const uint8_t pictofrag[NALLPICTS] = {
    [P_SCZP1]  = EX_SCZP1,
    [P_ASXP1]  = EX_ASXP1,
    [P_SWXP1]  = EX_SWXP1,
    [P_PRBP1]  = EX_PRBP1,
    [P_ASTP1]  = EX_ASTP2,
    [P_ASTP2]  = EX_ASTP2,
    [P_ASTP3]  = EX_ASTP4,
    [P_ASTP4]  = EX_ASTP4,
    [P_TIEP1]  = EX_TIEP1,
    [P_TIEP2]  = EX_TIEP1,
    [P_TIEP3]  = EX_TIEP1,
    [P_TIEP4]  = EX_TIEP1,
    [P_BXPIC]  = EX_BXPIC,
    [P_SWPIC1] = EX_SWPIC1,
    [P_LNDP1]  = EX_LNDP1,
    [P_LNDP2]  = EX_LNDP1,
    [P_LNDP3]  = EX_LNDP1,
    [P_UFOP1]  = EX_UFOP1,
    [P_UFOP2]  = EX_UFOP1,
    [P_UFOP3]  = EX_UFOP1,
    [P_TEREX]  = EX_TEREX,
    [P_PLAPIC] = EX_PLAPIC,
    [P_PLBPIC] = EX_PLBPIC,
};

static fragdesc_t *fragdesctab[EX__NUM];

static expl_t ramals[NRAMALS];

static uint32_t *plot_frag(const fragpat_t *fp, uint16_t x, uint8_t y)
{
    uint32_t *dst = (uint32_t *)SCRPTR(x, y);
    uint8_t shift = (x & 0xf) >> 1;
    
    dst[0] |= fp->pat[shift][0];
    dst[1] |= fp->pat[shift][1];
    dst[40] |= fp->pat[shift][2];
    dst[41] |= fp->pat[shift][3];

    return dst;
}
    
// Build a 2x2 frag
//    aa aa
//    bb bb
void mkfrag(fragpat_t *fp, uint8_t a, uint8_t b)
{
    uint32_t l00, l01, l10, l11;
    uint8_t i;

    l00 = l01 = l10 = l11 = 0;
    if (a & 0x10) l00 |= 0x80000000;
    if (a & 0x20) l00 |= 0x00008000;
    if (a & 0x40) l01 |= 0x80000000;
    if (a & 0x80) l01 |= 0x00008000;
    if (a & 0x01) l00 |= 0x40000000;
    if (a & 0x02) l00 |= 0x00004000;
    if (a & 0x04) l01 |= 0x40000000;
    if (a & 0x08) l01 |= 0x00004000;
    if (b & 0x10) l10 |= 0x80000000;
    if (b & 0x20) l10 |= 0x00008000;
    if (b & 0x40) l11 |= 0x80000000;
    if (b & 0x80) l11 |= 0x00008000;
    if (b & 0x01) l10 |= 0x40000000;
    if (b & 0x02) l10 |= 0x00004000;
    if (b & 0x04) l11 |= 0x40000000;
    if (b & 0x08) l11 |= 0x00004000;
    fp->pat[0][0] = l00;
    fp->pat[0][1] = l01;
    fp->pat[0][2] = l10;
    fp->pat[0][3] = l11;
    for (i = 1; i < 8; i++) {
        l00 >>= 2;
        l01 >>= 2;
        l10 >>= 2;
        l11 >>= 2;
        fp->pat[i][0] = l00;
        fp->pat[i][1] = l01;
        fp->pat[i][2] = l10;
        fp->pat[i][3] = l11;
    }
}

static void eerase(expl_t *expl)
{
    uint32_t **ep;
    uint32_t *wp;

    for (ep = expl->erases; ep != &expl->erasetab[EX_MAXFRAGS]; ep++) {
        wp = *ep;
        wp[0] = wp[1] = wp[40] = wp[41] = 0;
    }
    expl->erases = ep;
}

static void ewrite(expl_t *expl)
{
    const fragdesc_t *fd = expl->fragdesc;
    uint16_t rsize = ((expl->rsize + 0x100) & 0x7fff) >> 5;    // pre-scale
    uint8_t half_w = fd->w >> 1;
    uint8_t half_h = fd->h >> 1;
    int8_t r, offs_r;
    int8_t c, offs_c;
    uint16_t px, py;
    int16_t dx, dy;
    uint32_t *dst;
    uint8_t oddh = fd->h & 1;
    uint8_t oddw = fd->w & 1;

    offs_r = -half_h;
    for (r = 0; r < fd->h; r++) {
        if (offs_r == 0 && !oddh) {
            ++offs_r;
        }
        if (offs_r < 0) {
            dy = -(rsize >> (3 - -offs_r));
        } else {
            dy = rsize >> (3 - offs_r);
        }
        ++offs_r;
        offs_c = -half_w;
        for (c = 0; c < fd->w; c++) {
            offs_c = c - half_w;
            if (offs_c == 0 && !oddw) {
                ++offs_c;
            }
            if (offs_c < 0) {
                dx = -(rsize >> (3 - -offs_c));
            } else {
                dx = rsize >> (3 - offs_c);
            }
            ++offs_c;
            px = (expl->center_x + dx) << 1;
            py = expl->center_y + dy;
            if (px >= 16 && px < 304 &&
                py >= YMIN && py < YMAX) {
                dst = plot_frag(&fd->fragpat[c][r], px, py);
                *--expl->erases = dst;
            }
        }
    }
}

static void appear_finish(expl_t *expl)
{
    obj_t *obj = expl->objptr;

    expl->rsize = 0;
    obj->opict = expl->obdesc;
    obj->otyp &= 0xfd;      // clear not hyperable flag
    eerase(expl);
}

static void appear_update(expl_t *expl)
{
    obj_t *obj;
    int16_t dx;

    expl->rsize -= 0x100 * 6 / 5;
    if (expl->rsize >= 0) {  // finished appearing?
        appear_finish(expl);
        return;
    }

    obj = expl->objptr;
    dx = obj->ox16 - gd->bgl + 0xc00;
    if ((dx & 0xc000) != 0) { // on screen?
        // No, finish
        appear_finish(expl);
    } else {
        // Yes, continue
        dx = (dx - 0xc00) << 2;
        expl->toplft_x = dx >> 8;
        expl->toplft_y = obj->oy16 >> 8;
        expl->center_x = expl->toplft_x + (expl->obdesc->w >> 2);
        expl->center_y = expl->toplft_y + (expl->obdesc->h >> 1);
        eerase(expl);
        ewrite(expl);
    }
}

static void explosion_update(expl_t *expl)
{
    int16_t dx;

    expl->rsize += 0xaa * 6 / 5;
    if (expl->rsize > 0x3000) {
        // finished. kill it
        eerase(expl);
        expl->rsize = 0;
        return;
    }
    dx = (gd->bglx & 0xffc0) - (gd->bgl & 0xffc0);
    dx <<= 2;    // s10p6 -> s8p8
    expl->center_x += (dx >> 8);
    expl->toplft_x += (dx >> 8);
    eerase(expl);
    ewrite(expl);
}

void expu(void)
{
    expl_t *expl;

    for (expl = ramals; expl != &ramals[NRAMALS]; expl++) {
        if (gd->status & ST_APPEAREXPLDIS) {
            // not in play
            if (expl->rsize >= 0) {
                expl->rsize = 0;        // kill explosion
            } else {
                appear_finish(expl);    // finish appear
            }
        } else {
            if (expl->rsize < 0) {
                appear_update(expl);
            } else if (expl->rsize > 0) {
                explosion_update(expl);
            }
        }
    }
}

static expl_t *expl_alloc(void)
{
    expl_t *expl = gd->lsexpl;

    do {
        if (expl != NULL) {
            ++expl;
        }
        if (expl == NULL || expl == &ramals[NRAMALS]) {
            expl = &ramals[0];
        }
        if (expl == gd->lsexpl) {
            return NULL;
        }
    } while (expl->rsize < 0);
    // found a free slot
    if (expl->rsize != 0) {     // old explosion completed?
        eerase(expl);           // no, erase frags
    }
    return expl;
}

// appear start
void apst(obj_t *obj)
{
    const sprpat_t *opict = obj->opict;
    uint32_t opictid;
    uint16_t scrx16;
    uint8_t fragid;
    expl_t *expl;

    gd->optr = obj;
    scrx16 = obj->ox16 - gd->bgl;
    if (opict == &sprtab[P_NULOB] || scrx16 > 0x2600) {
        // nul patter or off-screen, abort appear
        return;
    }
    obj->opict = &sprtab[P_NULOB];
    expl = expl_alloc();
    if (expl == NULL) {
        // not slots available, give up
        obj->opict = opict;
        return;
    }
    if (!(gd->status & ST_GAMEOV)) {
        sndld(apsnd);
    }
    obj->otyp |= 0x2;           // not hyperable
    expl->rsize = 0xaf00;       // initial size
    expl->obdesc = opict;       // save the old pict
    expl->erases = &expl->erasetab[EX_MAXFRAGS]; // work backwards
    expl->objptr = obj;
    opictid = opict - sprtab;
    if (opictid >= NALLPICTS) {
        panic(0xf22399ee, opictid);
    }
    fragid = pictofrag[opictid];
    if (fragid == EX_NONE) {
        panic(0x1de332f5, opictid);
    }
    expl->fragdesc = fragdesctab[fragid];
}

// explosion start
void exst(obj_t *obj)
{
    const sprpat_t *opict = obj->opict;
    uint32_t opictid;
    uint16_t scrx16;
    uint8_t fragid;
    expl_t *expl;

    scrx16 = obj->ox16 - gd->bgl;
    if (opict == &sprtab[P_NULOB] || scrx16 > 0x2600) {
        return;
    }
    expl = expl_alloc();
    if (expl == NULL) {
        // no explosion for you
        return;
    }
    gd->lsexpl = expl;
    expl->rsize = 0x100;
    expl->obdesc = opict;
    expl->erases = &expl->erasetab[EX_MAXFRAGS]; // work backwards
    scrx16 <<= 2;       // 10p6 to 8p8
    // simplified...
    expl->toplft_x = scrx16 >> 8;
    expl->toplft_y = obj->oy16 >> 8;
    expl->center_x = expl->toplft_x + (expl->obdesc->w >> 2);
    expl->center_y = expl->toplft_y + (expl->obdesc->h >> 1);
    opictid = opict - sprtab;
    if (opictid >= NALLPICTS) {
        panic(0xf22399ee, opictid);
    }
    fragid = pictofrag[opictid];
    if (fragid == EX_NONE) {
        panic(0x1dd332f5, opictid);
    }
    expl->fragdesc = fragdesctab[fragid];
}

void expl_init(void)
{
    const pict_t *pict;
    const uint8_t *pat;
    uint16_t i, r, c, ir, ic;
    fragdesc_t *fd;
    fragpat_t *fp;

    for (i = 1; i < EX__NUM; i++) {
        pict = allpicts[fragtopic[i]];
        pat = pict->pat;
        r = pict->h / 2;
        c = pict->w;
        fd = malloc(sizeof(*fd) + c * sizeof(fragpat_t *));
        fd->w = c;
        fd->h = r;
        if (r * c > EX_MAXFRAGS) {
            panic(0xfe551123, r, c);
        }
        fp = malloc(sizeof(*fp) * r * c);
        for (ic = 0; ic < c; ic++) {
            fd->fragpat[ic] = fp;
            for (ir = 0; ir < r; ir++) {
                mkfrag(fp++, pat[0], pat[1]);
                pat += 2;
            }
        }
        fragdesctab[i] = fd;
    }
}
