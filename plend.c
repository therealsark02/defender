/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Player end (death, exploding).
 */

#include "gd.h"
#include "sprites.h"
#include "screen.h"
#include "plstrt.h"
#include "gexec.h"
#include "color.h"
#include "tdisp.h"
#include "attract.h"
#include "sinit.h"
#include "mess.h"
#include "sound.h"
#include "plend.h"

typedef struct {
    uint32_t *peradst;
    uint32_t *peramask;
    uint8_t perashift;
    uint16_t pxpost;
    uint16_t pypost;
    int16_t pxvelt;
    int16_t pyvelt;
} pex_t;

#define PNBITS  128
static pex_t table[PNBITS];

static uint32_t pxmask[16][4];

static const uint8_t pxcol[] = {
    0xff, 0x7f, 0x3f, 0x37, 0x2f, 0x27, 0x1f, 0x17, 7, 6, 5, 4, 3, 2, 0
};

static const uint8_t pxctb[] = {
    7, 7, 7, 0xf, 0x3f, 0x7f, 0xff, 0xff, 0
};

static void px1a(void)
{
    proc_t *p = gd->crproc;
    uint16_t nx, ny;
    pex_t *tp;

    pcram[11] = *gd->pcolp;
    if (pcram[11] == 0) {
        // end... return as desired
        p->plex.func();
        panic(0x23feb411);
        /* not reached */
    }
    for (tp = table; tp < &table[PNBITS]; tp++) {
        // erase 2x2 pixels
        if (tp->peradst != NULL) {
            tp->peradst[0] &= ~tp->peramask[0];
            tp->peradst[1] &= ~tp->peramask[1];
            tp->peradst[40] &= ~tp->peramask[0];
            tp->peradst[41] &= ~tp->peramask[1];
            if (tp->perashift == 15 && tp->pxpost < (303 << 7)) {
                tp->peradst[2] &= ~tp->peramask[2];
                tp->peradst[3] &= ~tp->peramask[3];
                tp->peradst[42] &= ~tp->peramask[2];
                tp->peradst[43] &= ~tp->peramask[3];
            }
        }
        tp->peradst = NULL;
        ny = tp->pypost + tp->pyvelt;
        if (ny >= ((YMIN - 2) << 8) && ny < ((YMAX + 2) << 8)) {
            tp->pypost = ny;
            nx = tp->pxpost + tp->pxvelt;
            if (ny >= (YMIN << 8) && ny < (YMAX << 8) &&
                nx >= (16 << 7) && nx < (304 << 7)) {
                tp->pxpost = nx;
                tp->peradst = (uint32_t *)SCRPTR(nx >> 7, ny >> 8);
                tp->perashift = (nx >> 7) & 0xf;
                tp->peramask = pxmask[tp->perashift];
                tp->peradst[0] |= tp->peramask[0];
                tp->peradst[1] |= tp->peramask[1];
                tp->peradst[40] |= tp->peramask[0];
                tp->peradst[41] |= tp->peramask[1];
                if (tp->perashift == 15 && tp->pxpost < (303 << 7)) {
                    tp->peradst[2] |= tp->peramask[2];
                    tp->peradst[3] |= tp->peramask[3];
                    tp->peradst[42] |= tp->peramask[2];
                    tp->peradst[43] |= tp->peramask[3];
                }
            }
        }
    }
    if (--gd->pcolc == 0) {
        ++gd->pcolp;
        gd->pcolc = 4;
    }
    sleep(px1a, 1);
}

// x = 2px prec
static void plex(uint8_t x, uint8_t y, void (*func)(void))
{
    proc_t *p = gd->crproc;
    int16_t vel, absxv;
    uint8_t A, B;
    pex_t *tp;

    p->plex.func = func;
    gd->pcent_x = x;
    gd->pcent_y = y;
    gd->psed = 0x08;
    gd->lpsed = 0x08;
    gd->psed2 = 0x17;
    gd->lpsed2 = 0x32;

    // initialize pieces
    for (tp = table; tp < &table[PNBITS]; tp++) {
        tp->pxpost = gd->pcent_x << 8;
        tp->pypost = gd->pcent_y << 8;

        A = ((gd->lpsed >> 1) ^ gd->lpsed) >> 1;
        gd->lpsed = (gd->lpsed >> 1) | ((gd->psed & 1) ? 0x80 : 0);
        gd->psed = (gd->psed >> 1) | ((A & 1) ? 0x80 : 0);
        A = (gd->psed & 1) - 1;
        B = gd->lpsed;
        vel = (A << 8) | B;
        tp->pxvelt = vel * 6 / 5;
        if (vel < 0) {
            vel = ~vel;
        }
        absxv = vel;

        A = ((gd->lpsed2 >> 1) ^ gd->lpsed2) >> 1;
        gd->lpsed2 = (gd->lpsed2 >> 1) | ((gd->psed2 & 1) ? 0x80 : 0);
        gd->psed2 = (gd->psed2 >> 1) | ((A & 1) ? 0x80 : 0);
        A = (gd->psed2 & 3) - 2;
        B = gd->lpsed2;
        vel = (A << 8) | B;
        tp->pyvelt = vel;
        if (vel < 0) {
            vel = ~vel;
        }
        vel = (vel >> 1) + absxv;
        if (vel >= 0x16a) {
            --tp; /* get another set of randoms */
        }
        tp->peradst = NULL;
    }
    // player explosion
    gd->pcolp = pxcol;
    gd->pcolc = 56;
    sleep(px1a, 1);
}

static void ple02(void) __dead;
static void ple02(void)
{
    uint8_t pid = gd->curplr;
    pldata_t *pyr;

    do {
        if (++pid > gd->plrcnt) {
            pid = 1;
        }
        pyr = pldx(pid);
    } while (pyr->plas == 0);
    gd->curplr = pid;
    ++gd->pdflg;
    plstrt();
}

static void ple3(void)
{
    hallof();
}

static void ple01(void)
{
    pldata_t *pyr = gd->plrx;

    if (pyr->plas != 0) {
        ple02();
        /* not reached */
    }
    if (gd->plrcnt > 1) {
        pyr = pldx(gd->curplr ^ 3); // EOR 01 <-> 10
        if (pyr->plas != 0) {
            messf(0x3c << 1, 0x78,
                ID_PLAYER_GAME_OVER,
                (gd->curplr == 1) ? "ONE" : "TWO");
            sleep(ple02, 0x60 * 5 / 6);
        }
    }
    messf(0x3e * 2, 0x80, ID_GAME_OVER);
    gd->status = 0xff;
    gd->sndtmr = 0;
    sndout(0x13);
    sleep(ple3, 40 * 5 / 6);
    panic(0x55ee33cc);
}

static void pdth5c(void)
{
    sclr1();
    ple01();
}

static void pdth5b(void)
{
    scrtrn();
    gd->sndtmr = 0;
    sndout(0x13);
    if (wvchk() == 0) {
        bonus(pdth5c);
        /* not reached */
    }
    ple01();
}

static void pdth5(void)
{
    proc_t *p = gd->crproc;

    coff(gd->nplascrx, gd->nplayc, p->plend.spr);
    pcram[0] = 0;
    gncide();
    plex(gd->nplaxc + 4, gd->nplayc + 3, pdth5b);
}

static void pdthl(void) __dead;

static void pdth2(void)
{
    proc_t *p = gd->crproc;
    uint8_t c;
    
    cwrit(gd->nplascrx, gd->nplayc, p->plend.spr);
    c = *p->plend.col++;
    if (c != 0) {
        pcram[11] = c;
        pcram[0] = 0;
        sleep(pdthl, 2);
        /* not reached */
    } else {
        gd->status = 0x7f;
        pcram[0] = 0xff;
        sleep(pdth5, 2);
    }
}

static void pdthl(void)
{
    proc_t *p = gd->crproc;
    
    coff(gd->nplascrx, gd->nplayc, p->plend.spr);
    sleep(pdth2, 2);
}

void plend(void)
{
    proc_t *p = gd->crproc;
    
    stchk0(0x58);
    gd->bglx = gd->bgl;
    blkclr(gd->plascrx, gd->playc, 16, 6);
    plsav();
    sndld(pdsnd);

    // player death!
    p->plend.col = pxctb;
    p->plend.spr = (gd->nplad < 0) ? &sprtab[P_PLBPICM] : &sprtab[P_PLAPICM];
    pdthl();
}

void plend_init(void)
{
    uint16_t *wp;
    uint16_t *rp;
    uint16_t shift;

    wp = (uint16_t *)&pxmask[0][0];
    wp[0] = 0xc000;
    wp[1] = 0xc000;
    wp[2] = 0x0000;
    wp[3] = 0xc000;
    wp[4] = 0;
    wp[5] = 0;
    wp[6] = 0;
    wp[7] = 0;
    for (shift = 1; shift < 16; shift++) {
        rp = wp;
        wp += 8;
        wp[0] = rp[0] >> 1;
        wp[1] = rp[1] >> 1;
        wp[2] = rp[2] >> 1;
        wp[3] = rp[3] >> 1;
        wp[4] = ((rp[0] & 1) ? 0x8000 : 0) | (rp[4] >> 1);
        wp[5] = ((rp[1] & 1) ? 0x8000 : 0) | (rp[5] >> 1);
        wp[6] = ((rp[2] & 1) ? 0x8000 : 0) | (rp[6] >> 1);
        wp[7] = ((rp[3] & 1) ? 0x8000 : 0) | (rp[7] >> 1);
    }
}

