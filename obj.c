/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Object handling. An 'object' is an on-screen object with a picture (image),
 * x & y position, x & y velocity, scanner blip, and collision action function.
 * Objects are usually associated with a process.
 * Objects are held in either a free list, an active list (for objects on the
 * screen (or in its vicinity), or an inactive list (for objects off-screen).
 * Thinks like collision checks and smart-bomb effects only apply to objects
 * in the active list.
 */

#include "gd.h"
#include "screen.h"
#include "sprites.h"
#include "irq_thread.h"
#include "tdisp.h"
#include "sound.h"
#include "expl.h"
#include "obj.h"

obj_t olist[NOBJS];

void oinit(void)
{
    uint8_t flags, i;

    flags = irq_thread_disable();
    gd->ofree = &olist[0];
    for (i = 0; i < NOBJS - 1; i++) {
        olist[i].olink = &olist[i + 1];
    }
    olist[i].olink = NULL;
    gd->iptr = NULL;
    gd->optr = NULL;
    gd->sptr = NULL;
    irq_thread_restore(flags);
}

static void obj_draw(obj_t *obj)
{
    const sprpat_t *spr = obj->opict;
    uint16_t x = obj->scrx;
    uint8_t y = obj->objy;
    uint8_t w = spr->w;
    uint8_t shift = x & 0xf;
    const uint16_t *src;
    uint16_t *dst;

    src = spr->pat[shift];
    dst = SCRPTR(x, y);

    if (x < 16) {
        // partially visible off screen left
        dst += 4;
        obj->erafn = spr->erase_right;
        spr->draw_right(dst, src);
    } else if (x + w > 304) {
        // partially visible off screen right
        obj->erafn = spr->erase_left;
        spr->draw_left(dst, src);
    } else {
        // fully visible
        if ((16 - shift) < w) {
            // spans 2 cols
            obj->erafn = spr->erase_full;
            spr->draw_full(dst, src);
        } else {
            // spans 1 col
            obj->erafn = spr->erase_left;
            spr->draw_left(dst, src);
        }
    }
    obj->eradst = dst;
    obj->erasrc = src;
}

void oproc(void)
{
    uint16_t scrx16;
    obj_t *obj;
    uint8_t y;

    if (gd->status & ST_NOSTOBJS) {
        // stars, objects, shells disabled
        return;
    }
    for (obj = gd->optr; obj != NULL; obj = obj->olink) {
        // off old object
        if (obj->objy != 0) {
            // object was drawn
            if (obj->objy <= gd->pminy || obj->objy > gd->pmaxy) {
                continue; // skip it
            }
            if (obj->eradst == NULL) {
                panic(0x8900ff, obj, obj->objy);
            }
            //eorcolor(0x007);
            obj->erafn(obj->eradst, obj->erasrc);
            obj->eradst = NULL;
            //eorcolor(0x007);
            obj->objx = obj->objy = 0; // mark erased;
        }

        // on new object
        y = u8p8_to_u8(obj->oy16);     // y = int(obj->oy16)
        if (y > gd->pminy && y <= gd->pmaxy) {
            // check on screen
            scrx16 = obj->ox16 - gd->bgl;
            if (scrx16 < screenx_to_s10p6(304)) {
                obj->scrx = s10p6_to_screenx(scrx16);    // 1px prec
                obj->objx = s10p6_to_u8(scrx16); // 2px prec
                if (obj->scrx + obj->opict->w > 16) {
                    obj->objy = y;
                    //eorcolor(0x070);
                    obj_draw(obj);
                    //eorcolor(0x070);
                }
            }
        }
    }
}

void velo(void)
{
    obj_t *obj;

    if (gd->status & ST_NOSTOBJS) {
        // stars, objects, shells disabled
        return;
    }
    for (obj = gd->optr; obj != NULL; obj = obj->olink) {
        obj->ox16 += obj->oxv;
        obj->oy16 += obj->oyv;
        if (obj->oy16 < YMIN_u8p8) {
            obj->oy16 = YMAX_u8p8;
        } else if (obj->oy16 > YMAX_u8p8) {
            obj->oy16 = YMIN_u8p8;
        }
    }
}

// alloc an object.
// caller must write to gd->optr to link it into the
// the allocated list
obj_t *getob(void)
{
    obj_t *obj = gd->ofree;

    if (obj == NULL) {
        error();
    }
    gd->ofree = obj->olink;
    obj->olink = gd->optr;
    obj->objx = 0;
    obj->objy = 0;
    obj->otyp = 0;
    return obj;
}

obj_t *obinit(proc_t *p, const sprpat_t *opict,
        uint8_t (*ocvect)(obj_t *obj),
        const struct blip_s *objcol)
{
    obj_t *obj = getob();
    obj->objy = 0; // mark erased
    obj->objid = p;
    obj->opict = opict;
    obj->ocvect = ocvect;
    obj->objcol = objcol;

    return obj;
}

uint8_t nokill(obj_t *obj)
{
    return 0;
}

void killop(obj_t *obj)
{
    kill(obj->objid);
}

void kilos(obj_t *obj, uint16_t scr, const uint8_t *snd)
{
    killob(obj);
    score(scr);
    ofshit(obj);
    exst(obj);
    sndld(snd);
}

void kilpos(obj_t *obj, uint16_t pts, const uint8_t *snd)
{
    killop(obj);
    kilos(obj, pts, snd);
}

static uint8_t _killob(obj_t **head, obj_t *obj)
{
    obj_t **plink;
    obj_t *o;

    o = (obj_t *)head;
    while ((plink = &o->olink, o = o->olink) != NULL) {
        if (o == obj) {
            *plink = o->olink;
            o->olink = gd->ofree;
            gd->ofree = o;
            return 1;
        }
    }
    return 0;
}

void killob(obj_t *obj)
{
    if (_killob(&gd->optr, obj) == 0) {
        if (_killob(&gd->iptr, obj) == 0) {
            error();
        }
    }
}

void kilshl(obj_t *obj)
{
    if (_killob(&gd->sptr, obj) == 0) {
        error();
    }
}

void ofshit(obj_t *obj)
{
    if (obj->eradst != NULL) {
        obj->erafn(obj->eradst, obj->erasrc);
        obj->eradst = NULL;
    }
}

void kiloff(obj_t *obj)
{
    killob(obj);
    ofshit(obj);
}

/*
 * scan active objects for off screen area
 */
void oscan(void)
{
    uint16_t xtemp = gd->bgl - 100 * 32;
    obj_t *obj, **plink;

    obj = (obj_t *)&gd->optr;
    while ((plink = &obj->olink, obj = obj->olink) != NULL) {
        if (obj->ox16 - xtemp >= 500 * 32) {
            *plink = obj->olink;
            obj->olink = gd->iptr;
            gd->iptr = obj;
            obj = (obj_t *)plink;
        }
    }
}

/*
 * scan inactive objects for on screen
 */
void iscan(void)
{
    uint16_t xtemp = gd->bgl - 100 * 32;
    obj_t *obj, **plink;
    uint16_t noy16;

    obj = (obj_t *)&gd->iptr;
    while ((plink = &obj->olink, obj = obj->olink) != NULL) {
        noy16 = obj->oy16 + obj->oyv * 8;
        if (noy16 < YMIN_u8p8) {
            noy16 = YMAX_u8p8;
        } else if (noy16 > YMAX_u8p8) {
            noy16 = YMIN_u8p8;
        }
        obj->oy16 = noy16;
        obj->ox16 += obj->oxv * 8;
        if ((obj->ox16 - xtemp) < 500 * 32) {
            *plink = obj->olink;
            obj->olink = gd->optr;
            gd->optr = obj;
            obj = (obj_t *)plink;
        }
    }
}

