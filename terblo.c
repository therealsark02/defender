/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Terrain blow-up (when you lose all your asronauts).
 */

#include "gd.h"
#include "obj.h"
#include "sprites.h"
#include "screen.h"
#include "scanner.h"
#include "mterr.h"
#include "bgalt.h"
#include "tdisp.h"
#include "color.h"
#include "ground.h"
#include "rand.h"
#include "sound.h"
#include "expl.h"
#include "terblo.h"

static void tblp0(void) __dead;
static void tbl3(void) __dead;

static void tblp0(void)
{
    obj_t *obj;
    uint8_t i;

    obj = gd->ofree;
    obj->opict = &sprtab[P_TEREX];
    for (i = 0; i < 2; i++) {
        obj->ox16 = ((rand() & 0x3f) << 8) + gd->bgl;
        obj->oy16 = getalt(obj->ox16) << 8;
        exst(obj);
    }
    pcram[0] = coltab[gd->seed & 0x1f];
    sndld(ahsnd);
    gd->ovcnt = 8;
    sleep(tbl3, 2);
}

static void tbl4(void)
{
    proc_t *p = gd->crproc;

    if (++p->terblo.cnt != 16) {
        tblp0();
    }
    sndld(tbsnd);
    sucide();
}

static void tbl3(void)
{
    proc_t *p = gd->crproc;

    pcram[0] = 0;
    gd->ovcnt = 8;
    sleep(tbl4, rmax((p->terblo.cnt >> 3) + 1));
}

void terblo(void)
{
    proc_t *p = gd->crproc;

    gd->status |= ST_NOTERRAIN;
    p->terblo.cnt = 0;
    bgeras();
    blkclr(SCANER_X, SCANER_Y + 0x20 - MTERRH, 128, MTERRH);

    pcram[0] = coltab[gd->seed & 0x1f];
    sndld(ahsnd);
    gd->ovcnt = 8;
    sleep(tbl3, 2);
}

