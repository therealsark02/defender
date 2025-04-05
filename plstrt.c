/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Player start, game start, wave parameters, enemy restoration, player
 * one vs. two switching.
 */

#include "gd.h"
#include "sprites.h"
#include "screen.h"
#include "color.h"
#include "scanner.h"
#include "obj.h"
#include "prdisp.h"
#include "player.h"
#include "gexec.h"
#include "exec.h"
#include "schitzo.h"
#include "rand.h"
#include "swarmer.h"
#include "stars.h"
#include "mess.h"
#include "tdisp.h"
#include "probe.h"
#include "tie.h"
#include "sound.h"
#include "astro.h"
#include "plstrt.h"

pldata_t pldata[2];

static const uint8_t wctab[] = {
    0x81, 0x28, 0x07, 0x16, 0x2f, 0x84, 0x15, 0
};

void plsav(void)
{
    pldata_t *pyr = gd->plrx;
    uint8_t *dst;
    int i;

    // zero ptarg and penemy[]
    pyr->ptarg = 0;
    memzero(pyr->penemy, sizeof(pyr->penemy));

    // save astros
    pyr->ptarg = gd->astcnt;

    // save enemy counts
    dst = pyr->penemy;
    memcpy(dst, &gd->lndres, &gd->lndcnt - &gd->lndres);
    // .. with counts
    for (i = 0; i < 5; i++) {
        dst[i] += *(&gd->lndcnt + i);
    }
}

void plres_astros(void)
{
    proc_t *p = mkproc(astro, STYPE);
    pldata_t *pyr = gd->plrx;
    uint8_t astcnt;
    obj_t **tgt;
    int i;

    tgt = p->astro.tgt = tlist;
    memzero(tlist, sizeof(tlist));

    astcnt = gd->astcnt = pyr->ptarg;
    if (gd->astcnt == 0) {
        return;
    }
    if (astcnt > 7) {
        // quadrant at a time
        for (i = 0; i < 4; i++) {
            astst(p, gd->astcnt >> 2, i * 0x40, tgt);
            tgt += gd->astcnt >> 2;
        }
        astcnt -= gd->astcnt & ~0x3;
    }
    while (astcnt--) {
        astst(p, 1, gd->hseed, tgt++);
    }
}

static void plres(void)
{
    pldata_t *pyr = gd->plrx;
    obj_t *obj;
    uint8_t n;

    // start astros
    plres_astros();

    // enemy counts
    memcpy(&gd->lndres, pyr->penemy, &gd->lndcnt - &gd->lndres);

    // clear active counts
    memzero(&gd->lndcnt, 5);

    // restore your swarmers
    while (gd->swmres != 0) {
        obj = getob();  // temp object
        obj->oy16 = ((gd->seed >> 1) + YMIN) << 8;
        obj->ox16 = (((rand() & 0x3f) + 0x80) << 8) + gd->bgl;
        n = (gd->swmres > 6) ? 6 : gd->swmres;
        mmsw(obj, n);
        obj->olink = gd->ofree; // kill the phony
        gd->ofree = obj;
        gd->swmres -= n;
    }

    // restore schitzos
    if (gd->sczres != 0) {
        sczst(gd->sczres);
        gd->sczres = 0;
    }

    // restore probes
    gd->prbcnt = gd->prbres;
    if (gd->prbres != 0) {
        gd->prbres = 0;
        prbst(gd->prbcnt);
    }

    // restore ties
    gd->tiecnt = gd->tieres;
    while (gd->tieres != 0) {
        n = gd->tieres;
        if (n > 3) {
            n = 3;
        }
        tiest(n);
        gd->tieres -= n;
    }
}

static void pls1(void)
{
    plres();
    stchk();
    gd->pdflg = 0;
    gexec();
}

static void pls01(void)
{
    pldata_t *pyr = gd->plrx;

    sclr1();
    stchka(0x5, pyr->ptarg);
    sleep(pls1, 1); // 0x60
}

static void plstr3(void)
{
    pldata_t *pyr;

    gd->bgl = 0;
    gd->bglx = 0;
    // alinit()
    // not required: bginit()
    sclr1();
    gd->pladir = PLADIR;
    gd->nplad = gd->pladir;
    gd->thflg = 0;
    gd->lflg = 0;
    gd->scrflg = 0;
    gd->revflg = 0;
    gd->sbflg = 0;
    gd->bmbcnt = 0;
    gd->tptr = &tlist[0];
    pyr = plindx();
    gd->plrx = pyr;
    pcram[5] = wctab[(pyr->pwav & 0x7) - 1];
    --pyr->plas;
    tdisp();
    gd->nplaxc = gd->plaxc = 0x20;
    gd->nplascrx = gd->plascrx = u8p8_to_screenx(u8_to_u8p8(gd->plaxc));
    gd->plax16 = u8_to_u8p8(gd->plaxc);
    gd->plabx = (0x2000 >> 2) + gd->bgl;
    gd->nplayc = gd->playc = 0x80;
    gd->play16 = u8_to_u8p8(gd->playc);
    gd->plaxv = 0;
    gd->playv = 0;
    mkproc(scproc, STYPE);
    mkproc(colr, STYPE);
    mkproc(flpup, STYPE);
    mkproc(plthrproc, STYPE);
    mkproc(cbomb, STYPE);
    mkproc(tiecol, STYPE);
    if (gd->pdflg == 0 || gd->plrcnt == 1) {
        // player start
        pls01();
    } else {
        messf(0x3c << 1, 0x80, ID_PLAYER_S,
            (gd->curplr == 1) ? "ONE" : "TWO");
        sleep(pls01, 0x80 * 5 / 6);
    }
}

static void plstr2(void) __dead;
static void plstr2(void)
{
    pinit();    // reinit proc list
    mkproc(plstr3, STYPE);
    exec();
}

static void plst1a(void)
{
    if (gd->lccnt || gd->rccnt) {
        // coins not finished, sleep
        sleep(plst1a, 1); // 15
    }
    plstr2();
}

void init20(void)
{
    if (!gd->startup) {
        crinit();
    }
    // not needed: laser related: fiss();
    stinit();
    oinit();
    // done earlier: fbinit();
    // done earlier: thinit();
}

void plstr0(void)
{
    init20();
    gncide();
    gd->status = 0x7f;
    if (gd->crproc != gd->active || gd->crproc->plink != NULL) {
        // I am not the only proc, so sleep
        sleep(plst1a, 1); // 15
    }
    plstr2();
}

// player start process
void plstrt(void)
{
    plstr0();
}

/*
 * wave data ** PAL adjusted **
 * max,min,intradelt,interdelt
 * w1,w2,w3,w4
 */
static const uint8_t wvtab[] = {
//          intradelt
//  max   min     interdelt w1    w2    w3    w4
    20,   0,    0,    0,    15,   20,   20,   20,       // landers
    3,    0,    0,    0,    0,    3,    4,    5,        // ties
    6,    0,    0,    0,    0,    1,    3,    4,        // probes
    10,   0,    0,    0,    0,    0,    0,    0,        // schitzos
    10,   0,    0,    0,    0,    0,    0,    0,        // swarmers
    30,   0,    0,    0,    25,   21,   17,   13,       // wave time
    5,    0,    0,    0,    5,    5,    5,    5,        // wave size
    0x60, 0,    3,    2,    0x1a, 0x24, 0x2e, 0x37,     // lander xv
    0x01, 0,    0,    0,    0x00, 0x00, 0x01, 0x01,     // lander yv msb
    0xff, 0,    0x10, 0x00, 0x86, 0xd3, 0x33, 0x33,     // lsb
    0x80, 0x10, 0xfc, 0xfe, 0x3e, 0x30, 0x23, 0x23,     // ldstim
    0x30, 0,    0,    0,    0x26, 0x30, 0x35, 0x3a,     // tie xv
    2,    0,    0,    0,    1,    1,    2,    2,        // szry
    1,    0,    0,    0,    0x00, 0x01, 0x01, 0x01,     // szyv msb
    0xff, 0x0,  0x08, 0x06, 0x76, 0x0d, 0x36, 0x49,     // " lsb
    0x60, 0x0,  0x8,  0x4,  0x0e, 0x22, 0x2b, 0x30,     // szxv
    0xff, 0x08, 0xfe, 0xfe, 0x23, 0x1c, 0x19, 0x17,     // szstim
    0x60, 0x0,  0x08, 0x02, 0x1a, 0x24, 0x26, 0x29,     // swxv
    40,   10,   -2,   -1,   21,   21,   21,   21,       // swstim
    0x3f, 0,    0,    0,    0x1f, 0x1f, 0x1f, 0x3f,     // swac
    0xc0, 0x18, 0xf4, 0xfc, 0xb1, 0xa3, 0x89, 0x7b,     // ufotim
    10,   3,    0xff, 0xff, 12,   11,   10,   8,        // ufstim
    200,  40,   0xf4, 0xf8, 240,  220,  200,  200,      // ufosk
};

/*
 * intra/inter-wall delta
 * col=2 for intra,3=inter
 * dst=penemy for inter-wall
 * dst=elist for intra-wall
 */
void wdelt(uint8_t *dst, uint8_t col)
{
    const int8_t *src = (int8_t *)wvtab;
    int16_t b;
    int i;

    for (i = 0; i < sizeof(wvtab) / 8; i++) {
        b = src[col];
        if (b < 0) {
            b += *dst;
            if (b < 0xff && b < src[0]) {
                *dst = b;
            }
        } else {
            b += *dst;
            if (b > -257 && b >= src[1]) {
                *dst = b;
            }
        }
        ++dst;
        src += 8;
    }
}

// get new wave params
void getwv(pldata_t *pyr)
{
    uint8_t restore_wave, col, difficulty, ceiling;
    const uint8_t *src;
    uint8_t *dst;
    int i;

    ++pyr->pwav;
    restore_wave = 5;      // GA4: RESTORE WAVE from cmos
    if (restore_wave != 0 && pyr->pwav % restore_wave == 0) {
        pyr->ptarg = 10;        // astros every Nth wave
    }
    // select the wvtab[] base column (1..4)
    col = (pyr->pwav > 4) ? 4 : pyr->pwav;

    // copy the selected wave column, w1 to w4, into pyr->penemy[]
    src = wvtab;
    col += 3;                 // cols 'w1' to 'w4'
    dst = pyr->penemy;
    for (i = 0; i < sizeof(wvtab) / 8; i++) {
        *dst++ = src[col];
        src += 8;
    }

    // apply difficulty
    difficulty = 5;         // GA1: INIT DIFFICULTY
    ceiling = 15;           // GA2: CEILING (in BCD)
    difficulty += (pyr->pwav >= 4) ? (pyr->pwav - 4) : 0;
    if (difficulty != 0) {
        if (difficulty > ceiling) {
            difficulty = ceiling;
        }
        // apply inter-wave deltas 'difficulty' times
        do {
            wdelt(pyr->penemy, 3);
        } while (--difficulty);
    }
}

static void start(void)
{
    pldata_t *pyr;

    gd->cunits = 0;         // coin stuff
    if (!gd->pwrflg) {
        return;
    }
    if (gd->status & ST_GAMEOV) {
        gncide();
        scrclr();
        gd->status = 0x7f;
        gd->curplr = 1;     // current player is player 1
        gd->pdflg = 1;      // display prompt
        gd->plrcnt = 0;     // will be incremented below
        memzero(pldata, sizeof(pldata));
        //rcmosa(&nship);
        pyr = &pldata[0];
        pyr->plas = 3;      // 3 ships (from CMOS NSHIP)
        pyr->psbc = 3;      // 3 smart bombs
        pyr->ptarg = 10;    // 10 astronauts
        gd->bunits = 0;     // coin stuff
        gd->jpad = 0;       // ensure no pending noise
        getwv(pyr);
        gd->repla = 0x100;     // from CMOS
        pyr->prpla = gd->repla << 8;
        pldata[1] = pldata[0];  // copy into second player
        mkproc(plstrt, STYPE);  // start player going
    }
    ++gd->plrcnt;
    --gd->credit;
    if ((gd->plrcnt - 1) == 0) {
        return;
    }
    tdisp();        // display score+lasers
}

// free-play check
static void fplay(void)
{
    gd->credit = 2;
}

// One Player Start
void st1(void)
{
    if (gd->status & ST_GAMEOV) {
        fplay();
        if (gd->credit != 0) {
            sndld(st1snd);
            start();
        }
    }
    sucide();
}

// Two Player Start
void st2(void)
{
    if (gd->status & ST_GAMEOV) {
        fplay();
        if (gd->credit >= 2) {
            start();
            sndld(st2snd);
            start();
        }
    }
    sucide();
}
