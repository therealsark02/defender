/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Probe (pod) object.
 */

#include "gd.h"
#include "obj.h"
#include "screen.h"
#include "sprites.h"
#include "blips.h"
#include "shell.h"
#include "expl.h"
#include "swarmer.h"
#include "rand.h"
#include "probe.h"

static uint8_t prbkil(obj_t *obj)
{
    kilos(obj, 0x210, prhsnd);
    mmsw(obj, rmax(6));
    --gd->prbcnt;
    return 1;
}

void prbst(uint8_t cnt)
{
    proc_t *p = gd->crproc;
    int16_t oyv;
    obj_t *obj;

    do {
        obj = obinit(p, &sprtab[P_PRBP1],
                    prbkil, &bliptab[B_CCCC]);
        rand();
        obj->ox16 = (hseed16() & 0x3fff) + 0x1000;
        obj->oy16 = ((gd->lseed >> 1) + YMIN) << 8;
        obj->oxv = (int16_t)(int8_t)((gd->seed & 0x3f) - 0x20);
        oyv = (int16_t)(int8_t)((gd->lseed & 0x7f) - 0x40);
        if (oyv >= 0) {
            oyv |= 0x20;
        } else {
            oyv &= 0xffdf;
        }
        obj->oyv = oyv;
        apst(obj);
    } while (--cnt);
}

