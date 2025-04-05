/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Tie (bomber) objects.
 */

#include "gd.h"
#include "obj.h"
#include "screen.h"
#include "sprites.h"
#include "blips.h"
#include "shell.h"
#include "tie.h"

static void bmbout(obj_t *shl)
{
    const sprpat_t *spr = gd->bax;
    uint16_t x = shl->scrx;
    uint8_t y = shl->objy;
    uint8_t shift = x & 0xf;
    const uint16_t *src;
    uint16_t *dst;

    src = spr->pat[shift];
    dst = SCRPTR(x, y);

    if (x < 16) {
        // partiall visible off screen left
        dst += 4;
        shl->erafn = spr->erase_right;
        spr->draw_right(dst, src);
    } else if (x > 304 - 3) {
        // partiall visible off screen right
        shl->erafn = spr->erase_left;
        spr->draw_left(dst, src);
    } else {
        // fully visible
        if ((16 - shift) < 3) {
            // spans 2 cols
            shl->erafn = spr->erase_full;
            spr->draw_full(dst, src);
        } else {
            // spans 1 col
            shl->erafn = spr->erase_left;
            spr->draw_left(dst, src);
        }
    }
    shl->eradst = dst;
    shl->erasrc = src;
}

static void bombst(obj_t *obj)
{
    obj_t *shl;

    if (gd->bmbcnt < 10) {
        shl = getshl(obj, bmbout, &sprtab[P_BMBP1], bkil);
        if (shl != NULL) {
            shl->odata[0] = (gd->seed & 0x1f) + 1; // lifetime
        }
    }
}

static void tie(void)
{
    sproc_t *p = (sproc_t *)gd->crproc;
    obj_t *obj;
    uint8_t A;
    int8_t B;

    obj = p->tie.tie[gd->seed & 3];
    if (obj == NULL) {
        sleep(tie, 1);
    }

    // new pict
    B = (gd->seed & 0x3f) - 0x20;
    if (B < 0) {
        if (--obj->opict < &sprtab[P_TIEP1]) {
            obj->opict = &sprtab[P_TIEP1];
        }
    } else {
        if (++obj->opict > &sprtab[P_TIEP4]) {
            obj->opict = &sprtab[P_TIEP4];
        }
    }
    obj->oyv += (int16_t)B;
    obj->oyv += -(obj->oyv >> 5);

    if (obj->objy == 0) {
        // off screen
        A = gd->seed;
        if (A <= 0x40) {
            // adjust cruise alt
            A = (A & 3) - 2 + p->tie.cralt;
            if (A < 0x40) {
                A = 0x40;
            } else if (A >= 0x68) {
                A = 0x68;
            }
            p->tie.cralt = A;
        }
        // y vel adjust
        A = p->tie.cralt - (obj->oy16 >> 8) + 0x10;
        if (A > 0x20) {
            if ((int8_t)(A - 0x10) >= 0) {
                obj->oyv -= 0x10;
            } else {
                obj->oyv += 0x10;
            }
        }
    } else {
        // on screen
        A = obj->objy - gd->playc;
        if ((int8_t)A >= 0) {
            // above player
            if (A >= 0x20) {
                // far
                obj->oyv -= 0x10;   // get closer
            } else {
                // close
                if (A <= 0x10) {
                    obj->oyv += 0x10;   // jink away
                }
            }
        } else {
            // below player
            if ((int8_t)A <= -0x20) {
                // far
                obj->oyv += 0x10;   // get closer
            } else {
                // close
                if ((int8_t)A > -0x10) {
                    obj->oyv -= 0x10;   // jink away
                }
            }
        }
        if ((gd->lseed & 7) == 0) {
            bombst(obj);
        }
    }
    sleep(tie, 1);
}

static uint8_t tiekil(obj_t *obj)
{
    sproc_t *p = (sproc_t *)obj->objid;
    uint16_t i;

    kilos(obj, 0x125, tihsnd);
    --gd->tiecnt;
    for (i = 0; i < 4; i++) {
        if (p->tie.tie[i] == obj) {
            p->tie.tie[i] = NULL;
        }
    }
    if (--p->tie.nsquad == 0) {
        kill((proc_t *)p);
    }
    return 1;
}

void tiest(uint8_t cnt)
{
    obj_t *obj;
    sproc_t *p;
    uint8_t xv;

    xv = gd->tiexv;
    gd->tflg = ~gd->tflg;
    if ((int8_t)gd->tflg >= 0) {
        xv = -xv;
    }
    p = msproc(tie, STYPE);
    p->tie.nsquad = cnt;
    p->tie.tie[0] = NULL;
    p->tie.tie[1] = NULL;
    p->tie.tie[2] = NULL;
    p->tie.tie[3] = NULL;
    do {
        obj = obinit((proc_t *)p, &sprtab[P_TIEP1],
                    tiekil, &bliptab[B_7777]);
        obj->oxv = (int16_t)(int8_t)xv;
        obj->oyv = 0;
        obj->ox16 = ((cnt + (cnt << 1)) << 7) + gd->plabx + 0x8000;
        obj->oy16 = 0x5000;
        p->tie.cralt = 0x50;
        obj->objid = (proc_t *)p;
        gd->optr = obj;
        p->tie.tie[cnt - 1] = obj;
    } while (--cnt);
}

