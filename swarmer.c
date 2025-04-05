/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Swarmer objects.
 */

#include "gd.h"
#include "obj.h"
#include "screen.h"
#include "sprites.h"
#include "blips.h"
#include "shell.h"
#include "expl.h"
#include "tdisp.h"
#include "rand.h"
#include "swarmer.h"

static uint8_t mswkil(obj_t *obj)
{
    --gd->swcnt;
    kiloff(obj);
    killop(obj);
    obj->ox16 -= 0x40;
    obj->oy16 -= 0x200;
    obj->opict = &sprtab[P_SWXP1];
    exst(obj);
    score(0x115);
    sndld(swhsnd);
    return 1;
}

static void swbmb(obj_t *obj)
{
    proc_t *p = gd->crproc;
    obj_t *shl;

    if ((int16_t)((gd->plabx - obj->ox16) ^ (obj->oxv & 0xff00)) >= 0) {
        shl = getshl(obj, fbout, &sprtab[P_BMBP1], bkil);
        if (shl != NULL) {
            shl->oxv = obj->oxv << 3;
            sndld(swssnd);
            shl->oyv = (int16_t)((gd->playc << 8) - (shl->oy16 & 0xff00)) >> 5;
        }
    }
    p->swarmer.stim = rmax(gd->swstim);
}

static void mswlp(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->swarmer.obj;
    int8_t xv, yv;

    yv = (gd->playc >= (obj->oy16 >> 8)) ? p->swarmer.accel : -p->swarmer.accel;
    obj->oyv += (int16_t)yv;
    if (obj->oyv < -0x200) {
        obj->oyv = -0x200;
    } else if (obj->oyv > 0x200) {
        obj->oyv = 0x200;
    }
    // damping
    obj->oyv += (int16_t)(int8_t)(~obj->oyv >> 6);
    // random factor
    obj->oyv += (int16_t)(int8_t)((gd->seed & 0x1f) - 0x10);
    if ((uint16_t)(gd->plabx - obj->ox16 + (150 * 32)) > (300 * 32)) {
        // go back and get him
        xv = (gd->plabx >= obj->ox16) ? gd->swxv : -gd->swxv;
        obj->oxv = (int16_t)xv;
        sleep(mswlp, 3);
    }
    // fire?
    if (--p->swarmer.stim == 0) {
        swbmb(obj);
    }
    sleep(mswlp, 3);
}

static void mswm(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->swarmer.obj;
    int8_t xv;

    xv = (gd->plabx >= obj->ox16) ? gd->swxv : -gd->swxv;
    obj->oxv = (int16_t)xv;
    sleep(mswlp, 3);
}

static void randv(obj_t *obj)
{
    rand();
    obj->oyv = (int16_t)(int8_t)gd->seed << 1;
    obj->oxv = (int16_t)(int8_t)((gd->lseed & 0x3f) - 0x20);
}

void mmsw(obj_t *prb, uint8_t cnt)
{
    obj_t *obj;
    proc_t *p;

    do {
        if (gd->swcnt + 1 > 20) {
            return;
        }
        ++gd->swcnt;
        p = mkproc(mswm, STYPE);
        obj = obinit(p, &sprtab[P_SWPIC1],
                    mswkil, &bliptab[B_2424]);
        obj->ox16 = prb->ox16;
        obj->oy16 = prb->oy16;
        p->swarmer.obj = obj;
        obj->objid = p;
        randv(obj);
        p->swarmer.accel = gd->lseed & gd->swac;
        p->ptime = gd->hseed & 0x1f;
        p->swarmer.stim = rmax(gd->swstim);
        gd->optr = obj;
    } while (--cnt);
}

