/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Astronaut objects.
 */

#include "gd.h"
#include "obj.h"
#include "blips.h"
#include "sprites.h"
#include "expl.h"
#include "tdisp.h"
#include "player.h"
#include "rand.h"
#include "terblo.h"
#include "sound.h"
#include "bgalt.h"
#include "astro.h"

static void alnd00(obj_t *obj, void (*paddr)(void)) __dead;

obj_t *tlist[NTLIST];

uint8_t astkil(obj_t *obj)
{
    if (!gd->pcflg) {
        return astk1(obj);
    }
    return 0;
}

static void p503(void)
{
    proc_t *p = gd->crproc;

    kiloff(p->p500.obj);
    sucide();
}

static void p5000(obj_t *obj, uint16_t scr)
{
    proc_t *p = gd->crproc;
    obj_t *aobj = p->astro.obj;

    score(scr);
    obj->oxv = gd->plaxv >> 8;
    obj->oyv = 0;
    obj->otyp = 0x11;
    obj->ox16 = aobj->ox16;
    if (aobj->oy16 & 0x8000) {
        obj->oy16 = aobj->oy16 - 0x2000;
    } else {
        obj->oy16 = aobj->oy16 + 0x1800;
    }
    gd->optr = obj;
    p->p500.obj = obj;
    sleep(p503, 42);
}

static void p500(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj;

    obj = obinit(p, &sprtab[P_C5P1], nokill, NULL);
    p5000(obj, 0x150);
}

static void p250(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj;

    obj = obinit(p, &sprtab[P_C25P1], nokill, NULL);
    p5000(obj, 0x125);
}

static void alnd00(obj_t *obj, void (*paddr)(void))
{
    proc_t *np;

    // landed; not fatal
    np = mkproc(paddr, STYPE);
    np->p500.obj = obj;
    obj->objid = NULL;
    obj->oyv = 0;
    obj->ocvect = astkil;
    sndld(alsnd);
    sucide();
}

static void afall2(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->astro.obj;

    obj->oyv = 0;
    obj->oy16 = gd->play16 + 0xa00;
    obj->ox16 = gd->plabx + 0x80;
    if (getalt(obj->ox16) >= u8p8_to_u8(obj->oy16)) {
        sleep(afall2, 1);
        /* not reached */
    }
    alnd00(obj, p500);
}

/*
 * astronaut fall
 */
void afall(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->astro.obj;
    int16_t nv;
    
    // accelerate down
    nv = obj->oyv + 10;     // was 8, scaled for PAL
    if (nv < PLADIR) {
        obj->oyv = nv;
    }
    if (getalt(obj->ox16) > u8p8_to_u8(obj->oy16)) {
        // still falling
        sleep(afall, 4);
        /* not reached */
    }
    // hit ground
    if (obj->oyv > 0x10c) {  // was 0xe0, scaled for PAL
        // fatal
        gd->centmpx = obj->objx + 1;
        gd->centmpy = obj->objy + 7;
        astk1(obj);
        sucide();
        /* not reached */
    }
    // landed; not fatal
    alnd00(obj, p250);
}

uint8_t akil1(obj_t *obj)
{
    proc_t *p = obj->objid;
    proc_t *np;

    if (p == NULL) {
        // not falling
        return astkil(obj);
    }
    if (gd->pcflg) {
        if (p->paddr != afall2) {
            sndld(acsnd);
            np = mkproc(p500, STYPE);
            np->p500.obj = obj;
        }
        p->paddr = afall2;
        return nokill(obj);
    }
    astk1(obj);
    killop(obj);
    return 1;
}

static void astclr(obj_t *obj)
{
    uint8_t i;

    for (i = 0; i < NTLIST; i++) {
        if (tlist[i] == obj) {
            tlist[i] = NULL;
            if (--gd->astcnt == 0) {
                mkproc(terblo, STYPE);
            }
            return;
        }
    }
    error();
}

uint8_t astk1(obj_t *obj)
{
    astclr(obj);
    kiloff(obj);
    obj->opict = &sprtab[P_ASXP1];
    obj->ox16 -= s8_to_s10p6(1);
    exst(obj);
    sndld(ahsnd);
    return 1;
}

/*
 * astronaut process
 * walk on terrain
 * pd =current index to tlist
 */
void astro(void)
{
    proc_t *p = gd->crproc;
    uint8_t alt, iy;
    obj_t *obj;

    if (++p->astro.tgt >= &tlist[16]) {
        p->astro.tgt = tlist;
    }
    obj = *p->astro.tgt;
    if (obj != NULL && obj->objy != 0 && obj->ocvect == astkil) {
        // exists, on-screen, not captured
        if (obj->opict == &sprtab[P_ASTP3] ||
            obj->opict == &sprtab[P_ASTP4]) {
            // walking right
            if (gd->seed <= 8) {
                // turn around
                obj->ox16 -= 0x20;   // $0.80 = 1 pixel
                obj->opict = &sprtab[P_ASTP1];
            } else {
                // move up/down with terrain
                alt = getalt(obj->ox16) + 15;
                if (alt > 0xe8) {
                    alt = 0xe8;
                }
                iy = u8p8_to_u8(obj->oy16);
                if (alt != iy) {
                    obj->oy16 += (alt < iy) ?
                        s8_to_s8p8(-1) : s8_to_s8p8(1);
                }
                // toggle image
                obj->opict = (obj->opict == &sprtab[P_ASTP3]) ?
                             &sprtab[P_ASTP4] : &sprtab[P_ASTP3];
                // move right
                obj->ox16 += 0x20;   // $0.80 = 1 pixel
            }
        } else {
            // walking left
            if (gd->seed <= 8) {
                // turn around
                obj->ox16 += 0x20;   // $0.80 = 1 pixel
                obj->opict = &sprtab[P_ASTP3];
            } else {
                // move up/down with terrain
                alt = getalt(obj->ox16) + 4;
                if (alt > 0xe8) {
                    alt = 0xe8;
                }
                iy = u8p8_to_u8(obj->oy16);
                if (alt != iy) {
                    obj->oy16 += (alt < iy) ?
                            s8_to_s8p8(-1) : s8_to_s8p8(1);
                }
                // toggle image
                obj->opict = (obj->opict == &sprtab[P_ASTP1]) ?
                             &sprtab[P_ASTP2] : &sprtab[P_ASTP1];
                // move left
                obj->ox16 -= 0x20;   // $0.80 = 1 pixel
            }
        }
    }
    sleep(astro, 2);
}

void astst(proc_t *p, uint8_t cnt, uint8_t hi_x, obj_t **tgt)
{
    obj_t *obj;

    while (cnt--) {
        obj = obinit(p, &sprtab[P_ASTP1], astkil, &bliptab[B_6666]);
        rand();
        obj->ox16 = (hseed16() & 0x1fff) + (hi_x << 8);
        if (!(obj->ox16 & 1)) {
            obj->opict = &sprtab[P_ASTP3]; // opposite direction
        }
        obj->oy16 = u8_to_u8p8(0xe0);
        obj->otyp = 0x10;       // non-collide
        obj->oyv = 0;
        obj->oxv = 0;
        obj->objid = NULL;
        gd->optr = obj;
        *tgt++ = obj;
    }
}

