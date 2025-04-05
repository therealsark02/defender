/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Schitzo (mutant) objects.
 */

#include "gd.h"
#include "screen.h"
#include "obj.h"
#include "blips.h"
#include "sprites.h"
#include "astro.h"
#include "expl.h"
#include "shell.h"
#include "rand.h"
#include "sound.h"
#include "bgalt.h"
#include "schitzo.h"

static void scz0(void) __dead;

// kill schitzo
static uint8_t sczkil(obj_t *obj)
{
    --gd->sczcnt;
    kilpos(obj, 0x115, schsnd);
    return 1;
}

/*
 * i am schitzo i,m feeling fine
 * pd=object,pd2=shot time
 */
static void scz0(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->schitzo.obj;
    uint16_t ud;
    int8_t sa;
    uint8_t ua;

    obj->oxv = (gd->plabx >= obj->ox16) ? gd->szxv : -gd->szxv;

    ud = gd->plabx - obj->ox16 + 0x380;
    if (ud > 0x700) {
        sa = gd->playc - u8p8_to_u8(obj->oy16);
        if (sa > 0) {
            obj->oyv = (sa > 8) ? 0 : -gd_szyv();
        } else {
            obj->oyv = (sa <= -8) ? 0 : gd_szyv();
        }
    } else {
        obj->oyv = (gd->playc < u8p8_to_u8(obj->oy16)) ? -gd_szyv() : gd_szyv();
        if (obj->objx == 0) {
            // not on screen
            sleep(scz0, 3);
            /* not reached */
        }
    }
    // random y hop
    ua = ((int8_t)gd->seed < 0) ? -gd->szry : gd->szry;
    ua += u8p8_to_u8(obj->oy16);
    if (ua < YMIN) {
        ua = YMAX;
    }
    obj->oy16 = u8_to_u8p8(ua);
    if (--p->schitzo.stim == 0) {
        p->schitzo.stim = rmax(gd->szstim);
        if (shoot(obj)) {
            sndld(sshsnd);
        }
    }
    sleep(scz0, 3);
}

void scz00(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj;

    --gd->lndcnt;
    ++gd->sczcnt;

    obj = p->lander.obj;
    obj->otyp = 0;      // ok to hyper
    obj->opict = &sprtab[P_SCZP1];
    obj->objcol = &bliptab[B_CC33];
    obj->ocvect = sczkil;
    p->schitzo.stim = gd->szstim;
    scz0();
    /* not reached */
}

void sczst(uint8_t cnt)
{
    uint16_t xtemp2, d;
    proc_t *p;
    obj_t *obj;

    do {
        p = mkproc(scz0, STYPE);
        obj = obinit(p, &sprtab[P_SCZP1], sczkil, &bliptab[B_CC33]);
        rand();
        xtemp2 = gd->bgl - (300 * 32);
        d = hseed16() - xtemp2;
        if (d < 600 * 32) {
            d += 0x8000;
        }
        d += xtemp2;        // avoid player
        obj->ox16 = d;
        obj->oy16 = u8_to_u8p8((gd->seed >> 1) + YMIN);
        obj->oyv = 0;
        obj->oxv = 0;
        // the code comment suggests PD2,U is the shot timer,
        // but the SCZST function erroneously writes it to PD,U
        // which is then overwritten by the obj pointer.
        // -- p->schitzo.stim = rmax(gd->szstim);
        apst(obj);
        obj->objid = p;
        p->schitzo.obj = obj;
        ++gd->sczcnt;
    } while (--cnt != 0);
}

