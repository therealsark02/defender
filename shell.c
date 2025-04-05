/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Shells (enemy bullets, mines).
 */

#include "gd.h"
#include "screen.h"
#include "obj.h"
#include "sprites.h"
#include "tdisp.h"
#include "expl.h"
#include "shell.h"

// get a shell
obj_t *getshl(obj_t *obj, void (*drawfn)(obj_t *shl),
        const struct sprpat_s *opict, uint8_t (*ocvect)(obj_t *shl))
{
    uint16_t scrx16;
    uint8_t A, B;
    obj_t *shl;

    if (gd->bmbcnt >= 20) {
        goto noshot;
    }
    // evaluate object screen position (s10p6)
    scrx16 = obj->ox16 - gd->bgl;
    if (scrx16 >= screenx_to_s10p6(304)) {
        goto noshot;
    }
    if (obj->oy16 <= YMIN_u8p8) {
        goto noshot;
    }
    shl = gd->ofree;
    if (shl == NULL) {
        goto noshot;
    }
    A = s10p6_to_u8(scrx16);    // int(screen_x)
    B = u8p8_to_u8(obj->oy16);  // int(y)
    shl->objx = A;
    shl->objy = B;
    shl->ox16 = (A << 8) | B;
    shl->oy16 = (B << 8) | A;
    shl->objid = obj->objid;
    shl->oxv = 0;
    shl->oyv = 0;
    shl->drawfn = drawfn;
    shl->opict = opict;
    shl->ocvect = ocvect;
    shl->odata[0] = 20; // default lifetime
    shl->odata[1] = 1;  // alive
    gd->ofree = shl->olink;
    shl->olink = gd->sptr;
    ++gd->bmbcnt;
    gd->sptr = shl;
    return shl;

noshot:
    return NULL;
}

// output shells
void shell(void)
{
    uint16_t nox16, noy16;
    int16_t dbgl;
    obj_t *shl;

    if (gd->status & ST_NOSTOBJS) {
        return;
    }
    // dbgl holds how much the background has moved, so the
    // screen x position of the shell can be corrected with a
    // simple add.
    // values & 0xffe0 to mask out the fractional pixels, then
    // the result is << 2 to transform from s10p6 -> s8.8
    dbgl = ((gd->bglx & 0xffe0) - (gd->bgl & 0xffe0)) << 2;

    shl = (obj_t *)&gd->sptr;
    while ((shl = shl->olink) != NULL) {
        // erase
        if (shl->eradst != NULL) {
            shl->erafn(shl->eradst, shl->erasrc);
            shl->eradst = NULL;
        }
        noy16 = shl->oy16 + shl->oyv;
        nox16 = shl->ox16 + dbgl - dbgl + shl->oxv + dbgl;
        if (nox16 >= screenx_to_u8p8(14) && nox16 < screenx_to_u8p8(304) && noy16 > YMIN_u8p8 && noy16 <= YMAX_u8p8) {
            shl->ox16 = nox16;                  // screen u8.8 2px
            shl->scrx = u8p8_to_screenx(nox16); // 1px
            shl->objx = u8p8_to_u8(nox16);      // 2px
            shl->oy16 = noy16;  // screen u8.8
            shl->objy = u8p8_to_u8(noy16);
            // plot
            shl->drawfn(shl);
        } else {
            // kill and erase
            shl->odata[1] = 0;
            if (shl->eradst != NULL) {
                shl->erafn(shl->eradst, shl->erasrc);
                shl->eradst = NULL;
            }
        }
    }
}

void fbout(obj_t *shl)
{
    const sprpat_t *spr = &sprtab[P_FIREBALL + gd->fbidx];
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

uint8_t bkil(obj_t *obj)
{
    score(0x25);
    --gd->bmbcnt;
    kilshl(obj);
    ofshit(obj);
    obj->ox16 = (obj->ox16 >> 2) + gd->bgl;
    obj->oy16 -= 0x200;
    obj->opict = &sprtab[P_BXPIC];
    exst(obj);
    sndld(ahsnd);
    return 1;
}

// shoot at player
uint8_t shoot(obj_t *obj)
{
    obj_t *shl = getshl(obj, fbout, &sprtab[P_FIREBALL + gd->fbidx], bkil);

    if (shl != NULL) {
        shl->oxv = (int16_t)(int8_t)((gd->seed & 0x1f) - 0x10 + gd->plaxc - shl->objx) << 2;
        if (gd->seed > 120) {   // rel?
            // yes - adjust ovx vs. player's
            shl->oxv = (gd->plaxv >> 6) + obj->oxv;
        }
        shl->oyv = (int16_t)(int8_t)((gd->lseed & 0x1f) - 0x10 + gd->playc - shl->objy) << 2;
        // scale velocities by 6/5 (1.2) for PAL
        shl->oxv = shl->oxv +
            (shl->oxv >> 3) +
            (shl->oxv >> 4);  // act. x1.1875
        shl->oyv = shl->oyv +
            (shl->oyv >> 3) +
            (shl->oyv >> 4);  // act. x1.1875
        return 1;
    }
    return 0;
}

/*
 * scan shell list for inactiveS
 */
void shscan(void)
{
    obj_t *shl, **plink;

    shl = (obj_t *)&gd->sptr;
    while ((plink = &shl->olink, shl = shl->olink) != NULL) {
        if (shl->odata[1] == 0 || --shl->odata[0] == 0) {
            *plink = shl->olink;
            shl->olink = gd->ofree;
            gd->ofree = shl;
            ofshit(shl);
            --gd->bmbcnt;
            shl = (obj_t *)plink;
        }
    }
}

