/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Smart bomb action.
 */

#include "gd.h"
#include "obj.h"
#include "sprites.h"
#include "plstrt.h"
#include "tdisp.h"
#include "sound.h"
#include "color.h"

static void sbx2a(void)
{
    gd->sbflg = 0;
    sucide();
}

static void sbx1a(void)
{
    if (gd->pia21 & 0x4) {
        sleep(sbx1a, 12);
        /* not reached */
    }
    sleep(sbx2a, 10);
}

static void sbmbx1(void)
{
    proc_t *p = gd->crproc;

    if (--p->sbomb.cnt != 0) {
        pcram[0] ^= 0xff;
        sleep(sbmbx1, 2);
        /* not reached */
    }
    sleep(sbx1a, 12);
}

void sbomb(void)
{
    pldata_t *pyr = gd->plrx;
    proc_t *p = gd->crproc;
    obj_t *obj;

    if (gd->sbflg || pyr->psbc == 0) {
        sucide();
    }
    ++gd->sbflg;
    if (!(gd->cheats & CHEAT_SBMAGIC)) {
        --pyr->psbc;
    }
    sbdisp();
    sndld(sbsnd);
    obj = gd->optr;
    while (obj != NULL) {
        if ((!(gd->cheats & CHEAT_SBMAGIC) && obj->objy == 0) || obj->otyp >= 2) {
            obj = obj->olink;
        } else {
            obj->ocvect(obj);
            obj = gd->optr;
        }
    }
    p->sbomb.cnt = 4;
    pcram[0] ^= 0xff;
    sleep(sbmbx1, 2);
}

