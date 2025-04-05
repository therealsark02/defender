/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Game process. Runs the game timer and dispatches enemies.
 */

#include "gd.h"
#include "plstrt.h"
#include "rand.h"
#include "screen.h"
#include "sprites.h"
#include "tdisp.h"
#include "lander.h"
#include "ufo.h"
#include "color.h"
#include "mess.h"
#include "gexec.h"

static void bc1(void) __dead;
static void bclp(void) __dead;

static void bclp(void)
{
    proc_t *p = gd->crproc;

    cwrit(p->bonus.x, 0xa0, &sprtab[P_ASTP3]);
    p->bonus.x += 8;
    score(0x100 | (((gd->plrx->pwav > 5) ? 5 : gd->plrx->pwav) << 4));
    sleep(bc1, 4);
}

static void bc1(void)
{
    proc_t *p = gd->crproc;

    if (--p->bonus.astcnt != 0) {
        bclp();
        /* not reached */
    }
    getwv(gd->plrx);
    sleep(p->bonus.func, 0x80 * 5 / 6);
    /* not reached */
}

static uint8_t hexbcd(uint8_t val)
{
    while (val >= 100) {
        val -= 100;
    }
    return ((val / 10) << 4) | (val % 10);
}

void bonus(void (*func)(void))
{
    uint8_t pwav = gd->plrx->pwav;
    proc_t *p = gd->crproc;

    pcram[0] = 0;
    p->bonus.func = func;           // return function
    sclr1();
    messf(0x38 * 2, 0x50, ID_ATTACK_WAVE_B, hexbcd(pwav));
    messf(0x3d * 2, 0x60, ID_COMPLETED);
    messf(0x3c * 2, 0x90, ID_BONUS_X, ((pwav > 5) ? 5 : pwav) << 8);

    p->bonus.x = 0x3c * 2;
    p->bonus.astcnt = gd->astcnt;
    if (p->bonus.astcnt != 0) {
        bclp();
        /* not reached */
    }
    getwv(gd->plrx);
    sleep(p->bonus.func, 0x80 * 5 / 6);
}

/*
 * game exec
 * checks end of wave
 * stages attacks
 * dispatches spoilers
 */

// check end of wave
uint8_t wvchk(void)
{
    return gd->lndcnt + gd->lndres + gd->tiecnt + gd->prbcnt +
           gd->swcnt + gd->sczcnt + gd->sczres;
}

static void gexeol(void)
{
    ++gd->plrx->plas;
    plstr0();
}

static void gex0(void)
{
    proc_t *p = gd->crproc;
    uint8_t nmecnt, ntmr, lcnt;

    if ((gd->status & ST_PLDEAD) == 0) {
        nmecnt = wvchk();
        if (nmecnt == 0) {
            // end of level
            gd->status = 0x77;          // disable everything
            gncide();                   // kill all
            plsav();
            bonus(gexeol);
            /* no return */
        }
        // not end of level
        if (nmecnt <= 8) {
            ntmr = gd->ufotim >> 1;
            if (nmecnt <= 3) {
                ntmr >>= 1;
            }
            if (++ntmr < gd->ufotmr) {
                gd->ufotmr = ntmr;
            }
        }
        if (--gd->ufotmr == 0) {
            ntmr = gd->ufotim;
            if (nmecnt < 4) {
                ntmr = rmax(ntmr >> 2);
            }
            gd->ufotmr = ntmr;
            if (gd->ufocnt < 12) {
                ufost();
                ++gd->ufocnt;
            }
        }
        if ((--gd->wavtmr == 0 || gd->lndcnt == 0) && (gd->wavtmr = gd->wavtim, gd->lndres != 0) && gd->lndcnt < 8) {
            lcnt = gd->wavsiz;
            if (lcnt > gd->lndres) {
                lcnt = gd->lndres;
            }
            landst(lcnt);
            gd->lndres -= lcnt;
        }
    }
    if (gd->strcnt < 16) {
        ++gd->strcnt;
    }
    if (++gd->gtime > 240) {
        gd->gtime = 0;
    }
    if (--p->pd[0] == 0) {
        wdelt(&gd->lndres, 2);
        p->pd[0] = 40;
    }
    sleep(gex0, 15);
}

void gexec(void)
{
    proc_t *p = gd->crproc;

    p->pd[0] = 40;
    gd->ufotmr = gd->ufotim;
    gd->wavtmr = 1;
    gex0();
}

