/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Lander objects.
 */

#include "gd.h"
#include "screen.h"
#include "obj.h"
#include "blips.h"
#include "sprites.h"
#include "astro.h"
#include "schitzo.h"
#include "shell.h"
#include "expl.h"
#include "rand.h"
#include "bgalt.h"
#include "sound.h"
#include "lander.h"

static void lndsaa(void) __dead;
static void lands0(void) __dead;
static void landsa(void) __dead;
static void landg(void) __dead;
static void landf(void) __dead;
static void lndfxa(void) __dead;

static obj_t *gtarg(proc_t *p)
{
    obj_t **tptr;
    obj_t *tobj;

    if (gd->astcnt != 0) {
        tptr = gd->tptr;
        do {
            if (++tptr >= &tlist[NTLIST]) {
                tptr = &tlist[0];
            }
            tobj = *tptr;
            if (tobj != NULL) {
                gd->tptr = tptr;
                p->lander.tobj = tobj;
                p->lander.tptr = tptr;
                return tobj;
            }
        } while (tptr != gd->tptr);
    }
    return NULL;
            
}

// lander shoot
static void lshot(proc_t *p)
{
    if (--p->lander.stim == 0) {
        p->lander.stim = rmax(gd->ldstim);
        if (shoot(p->lander.obj) != 0) {
            sndld(lshsnd);
        }
    }
}

static void lndfxa(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;
    obj_t *tobj = p->lander.tobj;

    if (*p->lander.tptr == NULL) {
        // give up
        kiloff(obj);
        --gd->lndcnt;
        ++gd->lndres;
        sucide();
        /* not reached */
    }
    obj->oyv = tobj->oyv = 0;
    if (u8p8_to_u8(tobj->oy16) > u8p8_to_u8(obj->oy16)) {
        tobj->oy16 -= u8_to_u8p8(1);
        //original bug: CLRB; LDA #$12; JSR SNDOUT
        //this calls sndout with B=0, which is a no-op
        //sndout(0);  // omit for no-op
        sleep(lndfxa, 1);
        /* not reached */
    }
    // kill astro
    // center eatherling blow
    gd->centmpx = tobj->objx + 1;
    gd->centmpy = tobj->objy;
    astk1(tobj);
    scz00();
}

// flee
static void landf(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;

    if (obj->oy16 > u8_to_u8p8(YMIN + 8)) {
        lshot(p);
        sleep(landf, 4);
        /* not reached */
    }
    // all done; pull him inside
    sndld(lsksnd);
    lndfxa();
    /* not reached */
}

// kill lander
static uint8_t lkill(obj_t *obj)
{
    --gd->lndcnt;
    kilpos(obj, 0x115, lhsnd);
    return 1;
}

// kill kidnapping lander
static uint8_t lkil1(obj_t *obj)
{
    proc_t *p = obj->objid;
    obj_t *tobj = p->lander.tobj;
    proc_t *np;

    if (*p->lander.tptr != NULL) {
        np = mkproc(afall, STYPE);
        np->astro.obj = tobj;
        sndld(ascsnd);
        tobj->oyv = 0;
        tobj->objid = np;
    }
    return lkill(obj);
}

static void landg(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;
    obj_t *tobj = p->lander.tobj;
    uint16_t tobjy, objy;
    int16_t xtemp2, D;

    if (*p->lander.tptr == NULL || tobj->ocvect != astkil) {
        // astro is dead or taken; choose another target
        lndsaa();
        /* not reached */
    }
    xtemp2 = tobj->ox16 & 0xffe0;
    D = obj->ox16 & 0xffe0;
    if (D != xtemp2) {
        obj->ox16 += (D < xtemp2) ? 0x20 : -0x20;
    }
    tobjy = u8p8_to_u8(tobj->oy16) - 12;
    objy = u8p8_to_u8(obj->oy16);
    if (tobjy != objy) {
        obj->oy16 += (tobjy < objy) ? -gd_lndyv() : gd_lndyv();
        lshot(p);
        sleep(landg, 1);
        /* not reached */
    }
    // are we on him?
    if (obj->ox16 + 0x40 - tobj->ox16 > 0x80) {
        // nope
        lshot(p);
        sleep(landg, 1);
        /* not reached */
    }
    obj->ocvect = lkil1;    // change kill vector
    obj->oyv = tobj->oyv = -gd_lndyv();
    sndld(lpksnd);
    tobj->ocvect = akil1;
    // flee
    landf();
    /* not reached */
}

static void landsa(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;
    int16_t alt, oyv;

    alt = getalt(obj->ox16) - 50 - u8p8_to_u8(obj->oy16);
    if (alt <= 0) {
        if (alt >= -20) {
            oyv = 0;
        } else {
            oyv = -gd_lndyv();
        }
    } else {
        oyv = gd_lndyv();
    }
    obj->oyv = oyv;

    // appearing?
    if (obj->opict != &sprtab[P_NULOB]) {
        // no, shoot!
        lshot(p);
        // new pict
        if (++obj->opict > &sprtab[P_LNDP3]) {
            obj->opict = &sprtab[P_LNDP1];
        }
    }
    sleep(lands0, 5);
}

static void lndsaa(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;

    obj->otyp &= 0xfe;
    if (gtarg(p) == NULL) {
        // turn to a sczo
        scz00();
        /* not reached */
    }
    landsa();
}

static void lands0(void)
{
    proc_t *p = gd->crproc;
    obj_t *obj = p->lander.obj;
    obj_t *tobj = p->lander.tobj;

    // target alive?
    if (*p->lander.tptr != NULL) {
        // yes. already kidnapped?
        if (tobj->ocvect == astkil) {
            // no.
            // close to target?
            if (((obj->ox16 ^ tobj->ox16) & 0xfc00) == 0) {
                // yes. get him!
                ++obj->otyp;        // dont hyper me
                obj->oxv = 0;
                obj->oyv = 0;
                obj->opict = &sprtab[P_LNDP1];
                landg();
            } else {
                // no, keep looking
                landsa();
            }
            /* not reached */
        }
    }
    lndsaa();
    /* not reached */
}

void landst(uint8_t cnt)
{
    obj_t *obj;
    proc_t *p;

    do {
        if (gd->astcnt == 0) {
            sczst(cnt);
            return;
        }
        p = mkproc(lands0, STYPE);

        obj = obinit(p, &sprtab[P_LNDP1], lkill, &bliptab[B_4433]);
        rand();
        obj->ox16 = hseed16();
        obj->oy16 = u8_to_u8p8(YMIN + 2);
        obj->oyv = gd_lndyv();
        p->lander.stim = rmax(gd->ldstim);    // shooting time
        obj->oxv = rmax(gd->lndxv);
        if (obj->oxv & 0x1) {
            obj->oxv = -obj->oxv;
        }
        apst(obj);
        p->lander.obj = obj;
        gtarg(p);
        ++gd->lndcnt;
    } while (--cnt != 0);
}

