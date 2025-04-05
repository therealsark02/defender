/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Hyperspace action.
 */

#include "gd.h"
#include "obj.h"
#include "sprites.h"
#include "screen.h"
#include "blips.h"
#include "player.h"
#include "plstrt.h"
#include "plend.h"
#include "expl.h"
#include "rand.h"
#include "hyper.h"

static void hyp2(void)
{
    proc_t *p = gd->crproc;

    kiloff(p->hyper.obj);
    stchk();
    if (gd->lseed > 192) {
        plend();
        /* not reached */
    }
    sucide();
}

static void hyp02(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj;

    while ((obj = gd->sptr) != NULL) {
        kilshl(obj);
    }
    gd->bmbcnt = 0;
    gd->bgl = gd->bglx = seed16();
    if (gd->lseed & 1) {
        gd->nplaxc = 0x20;
        gd->nplad = PLADIR;
    } else {
        gd->nplaxc = 0x70;
        gd->nplad = -PLADIR;
    }
    gd->nplascrx = u8p8_to_screenx(u8_to_u8p8(gd->nplaxc));
    gd->plax16 = gd->nplaxc << 8;
    gd->play16 = ((gd->hseed >> 1) + YMIN) << 8;
    gd->nplayc = gd->play16 >> 8;
    gd->plaxv = 0;
    gd->playv = 0;
    stchk0(0x50);
    obj = obinit(p, &sprtab[P_PLAPIC], nokill, &bliptab[B_0000]);
    obj->oxv = obj->oyv = 0;
    obj->oy16 = gd->play16;
    obj->ox16 = (gd->plax16 >> 2) + gd->bgl;
    if (gd->nplad < 0) {
        obj->opict = &sprtab[P_PLBPIC];
    }
    p->hyper.obj = obj;
    apst(obj);
    sleep(hyp2, 0x28 * 5 / 6);
}

void hyper(void)
{
    if (gd->status & 0xfd) {
        sucide();
        /* not reached */
    }
    gd->status = 0x77;
    sclr1();
    sleep(hyp02, 13);
}

