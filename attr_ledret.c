/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Attract Mode: Animated aliens and points sequence.
 */

#include "gd.h"
#include "screen.h"
#include "data.h"
#include "blips.h"
#include "sprites.h"
#include "color.h"
#include "obj.h"
#include "expl.h"
#include "scanner.h"
#include "tdisp.h"
#include "mess.h"
#include "cmos.h"
#include "attract.h"

#define XMAN    0x1e00
#define YMAN    0xdb00
#define XSHIP   0x0800
#define YSHIP   0x5000

typedef struct textent_s {
    uint8_t x;
    uint8_t y;
    uint16_t str;
} textent_t;

typedef struct enmyent_s {
    uint8_t pic;
    uint8_t blip;
    uint16_t x;
    uint16_t y;
} enmyent_t;

static void scinit(void)
{
    gd->status = 0xff;
    oinit();
    scrclr();
    gd->bgl = 0;
    gd->bglx = 0;
    crinit();
    gd->status = 0xdb;
    gd->plaxc = 0x10;
    gd->playc = 0x30;
}

static obj_t *stilob(const struct sprpat_s *opict,
        int16_t x, uint16_t y)
{
    obj_t *obj = getob();

    obj->objy = 0; // mark erased
    obj->oxv = 0;
    obj->oyv = 0;
    obj->ox16 = x;
    obj->oy16 = y;
    obj->opict = opict;
    gd->optr = obj;

    return obj;
}

static const enmyent_t enmytb[] = {
    { P_LNDP1,  B_4433, 0x0900, 0x6000 },
    { P_SCZP1,  B_CC33, 0x1100, 0x6000 },
    { P_UFOP1,  B_3333, 0x1980, 0x6200 },
    { P_TIEP1,  B_7777, 0x0960, 0x9800 },
    { P_PRBP1,  B_CCCC, 0x1160, 0x9800 },
    { P_SWPIC1, B_2424, 0x19e0, 0x9a00 }
};

static void amod12(const enmyent_t *ep);

static void amod13(void)
{
    sleep(amodes, 0xff * 5 / 6);
}

static void bmode3(void)
{
    const enmyent_t *ep = gd->otabpt;

    if (ep != &enmytb[6]) {
        amod12(ep);
    }
    sleep(amod13, 0xff * 5 / 6);
}

static void bmode2(void)
{
    ++gd->texptr;
    sleep(bmode3, 0x20 * 5 / 6);
}

static void amod11(void)
{
    const enmyent_t *ep; 
    obj_t *obj;

    obj = gd->enemyp;
    kiloff(obj);
    obj = getob();
    exst(obj);
    ep = gd->otabpt;
    obj->ox16 = ep->x;
    obj->oy16 = ep->y;
    obj->oxv = 0;
    obj->oyv = 0;
    apst(obj);
    gd->otabpt = ++ep;
    sleep(bmode2, 0x20 * 5 / 6);
}

static void amod10(void)
{
    gd->nplaxc = gd->shiptr->objx;
    gd->nplayc = gd->shiptr->objy;
    gd->lasreol = (304 - 40) >> 1;
    mkproc(attr_lasr, AMTYPE);
    sleep(amod11, 0x17 * 5 / 6);
}

static void amod12(const enmyent_t *ep)
{
    obj_t *obj;

    obj = getob();
    obj->opict = &sprtab[ep->pic];
    obj->objcol = &bliptab[ep->blip];
    obj->ox16 = 0x1f00;
    obj->oy16 = 0xa000;
    obj->oyv = -0xc0 * 6 / 5;
    obj->oxv = 0;
    apst(obj);
    gd->otabpt = ep;
    gd->enemyp = obj;
    sleep(amod10, 0x5f * 5 / 6);
}

static void amode8(void)
{
    obj_t *obj;

    obj = gd->shiptr;
    obj->opict = &sprtab[P_PLAPIC];
    obj->oxv = obj->oyv = 0;
    gd->nplaxc = obj->objx;
    gd->nplayc = obj->objy;

    obj = gd->p500ob;
    kiloff(obj);
    amod12(enmytb);
}

static void amode7(void)
{
    obj_t *obj;

    obj = gd->p500ob;
    obj->oy16 = 0xe000;
    obj->ox16 = 0x1c00;

    obj = gd->manptr;
    obj->oyv = 0;

    obj = gd->shiptr;
    obj->opict = &sprtab[P_PLBPIC];
    obj->oxv = -0x40 * 6 / 5;
    obj->oyv = -0x180 * 6 / 5;
    sleep(amode8, (0x60 * 5 / 6) - 1);
}

static void amode5(void)
{
    obj_t *obj;

    obj = stilob(&sprtab[P_C5P1], 0x1dff, 0x9000);
    obj->objcol = NULL;
    gd->p500ob = obj;
    obj = gd->shiptr;
    obj->oxv = 0;
    obj->oyv = 0xc0 * 6 / 5;
    obj = gd->manptr;
    obj->ox16 = 0x1e80;
    obj->oy16 = 0xa2e0;
    obj->oyv = 0xc0 * 6 / 5;
    sleep(amode7, 0x50 * 5 / 6);
}

static void amode4(void)
{
    obj_t *obj = gd->manptr;

    obj->oyv += 11;
    if (--gd->manfrf != 0) {
        sleep(amode4, 2);
    }
    amode5();
}

static void amode3(void)
{
    obj_t *obj;

    obj = gd->enemyp;
    kiloff(obj);
    exst(obj);

    obj = gd->shiptr;
    obj->oxv = 0x40 * 6 / 5;
    obj->oyv = 0xd4 * 6 / 5;
    gd->manfrf = 0x2d * 5 / 6;
    obj = gd->manptr;
    obj->oyv = 0;
    amode4();
}

static void amode2(void)
{
    gd->nplaxc = gd->shiptr->objx;
    gd->nplayc = gd->shiptr->objy;
    gd->lasreol = (304 - 64) >> 1;
    mkproc(attr_lasr, AMTYPE);
    sleep(amode3, 0x15 * 5 / 6);
}

static void amode1(void)
{
    gd->enemyp->oyv = -0xb0 * 6 / 5;
    gd->manptr->oyv = -0xb0 * 6 / 5;
    sleep(amode2, 0xa0 * 5 / 6);
}

static const textent_t textab[] = {
    { 0x43, 0x30, ID_TT_SCANNER },
    { 0x1c, 0x70, ID_TT_LANDER  },
    { 0x3c, 0x70, ID_TT_MUTANT  },
    { 0x5f, 0x70, ID_TT_BAITER  },
    { 0x1c, 0xa8, ID_TT_BOMBER  },
    { 0x40, 0xa8, ID_TT_POD     },
    { 0x5c, 0xa8, ID_TT_SWARMER },
};

static void textp1(const textent_t *ep) __dead;
static void textp(void) __dead;

static void textp2(void)
{
    const textent_t *ep = gd->textmp;

    if (ep != gd->texptr) {
        textp1(ep);
    }
    textp();
}

static void textp1(const textent_t *ep)
{
    messf(ep->x << 1, ep->y, ep->str);
    gd->textmp = ++ep;
    sleep(textp2, 6 * 5 / 6);
}

static void textp(void)
{
    textp1(textab);
}

void ledret(void)
{
    obj_t *obj;

    gncide();
    scinit();
    gd->status = 0xd9;
    scores();
    mkproc(creds, AMTYPE);
    border();
    gd->texptr = &textab[1];
    mkproc(colr, AMTYPE);
    mkproc(cbomb, AMTYPE);
    mkproc(tiecol, AMTYPE);
    mkproc(scproc, AMTYPE);
    mkproc(textp, AMTYPE);
    obj = stilob(&sprtab[P_ASTP1], XMAN, YMAN);
    obj->objcol = &bliptab[B_6666];
    gd->manptr = obj;

    obj = stilob(&sprtab[P_PLAPIC], XSHIP, YSHIP);
    obj->objcol = 0;
    gd->shiptr = obj;

    obj = getob();
    obj->opict = &sprtab[P_LNDP1];
    obj->ox16 = 0x1da0;
    obj->oy16 = 0x4000;
    obj->oyv = 0xa0 * 6 / 5;
    obj->oxv = 0;
    obj->objcol = &bliptab[B_4433];
    apst(obj);
    gd->enemyp = obj;
    sleep(amode1, 0xe6 * 5 / 6);
}

