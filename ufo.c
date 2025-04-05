/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * UFO (baiter) objects.
 */

#include "gd.h"
#include "screen.h"
#include "obj.h"
#include "sprites.h"
#include "shell.h"
#include "expl.h"
#include "blips.h"
#include "sound.h"
#include "rand.h"
#include "ufo.h"

void ufonv0(obj_t *obj)
{
    uint8_t xtemp = 0x4d;   // PAL scaled
    uint8_t ytemp = 0x01;
    uint16_t d;
    uint8_t a;

    d = obj->ox16 - gd->plabx;
    if ((int16_t)d >= 0) {
        xtemp = -xtemp;
    }
    d += 20 * 32;
    if (d > 40 * 32) {
        obj->oxv = (int16_t)(int8_t)xtemp + (gd->plaxv >> 8);
    }
    a = (obj->oy16 >> 8) - gd->playc;
    if ((int8_t)a >= 0) {
        ytemp = -ytemp;
    }
    a += 10;
    if (a > 20) {
        obj->oyv = (int16_t)((ytemp << 8) + gd->playv) >> 1;
    }
}

static void ufonv(obj_t *obj)
{
    if (gd->seed > gd->ufosk) {
        ufonv0(obj);
    }
}

static void ufolp(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->ufo.obj;

    if (obj->opict != &sprtab[P_NULOB]) {
        if (--p->ufo.stim == 0) {
            p->ufo.stim = rmax(gd->ufstim);
            if (shoot(obj)) {
                sndld(ushsnd);
            }
        }
        if (++obj->opict > &sprtab[P_UFOP3]) {
            obj->opict = &sprtab[P_UFOP1];
            ufonv(obj);
        }
    }
    sleep(ufolp, 5);
}

static uint8_t ufokil(obj_t *obj)
{
    --gd->ufocnt;
    kilpos(obj, 0x120, ufhsnd);
    return 1;
}

void ufost(void)
{
    obj_t *obj;
    proc_t *p;

    p = mkproc(ufolp, STYPE);
    obj = obinit(p, &sprtab[P_UFOP1], ufokil, &bliptab[B_3333]);
    p->ufo.obj = obj;
    obj->objid = p;
    obj->ox16 = (seed16() & 0x1fff) + gd->bgl;
    obj->oy16 = ((gd->lseed >> 1) + YMIN) << 8;
    obj->oyv = 0;
    obj->oxv = 0;
    p->ufo.stim = 7;
    ufonv0(obj);
    apst(obj);
}

