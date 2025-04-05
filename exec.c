/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Top-level process execute loop and switch action processing.
 */

#include "gd.h"
#include "obj.h"
#include "data.h"
#include "rand.h"
#include "swtch.h"
#include "collide.h"
#include "screen.h"
#include "plstrt.h"
#include "plend.h"
#include "joypad.h"
#include "cmos.h"
#include "keys.h"
#include "expl.h"
#include "exec.h"

uint32_t gd_timer_hist[8];

static void proc_timer(void)
{
    uint8_t timer;
    obj_t *pobj;
    obj_t *obj;
    int8_t n;

    do {
        timer = gd->timer;
        asm volatile("" ::: "memory");
    } while (timer == 0);
    if (gd->want_timing) {
        eorcolor(0x070);
    }
    gd->timer = 0;
    if (gd->status & 0x7d) {
        gd->ovcnt = 0;
        return;
    }
    if (timer < 8) {
        ++gd_timer_hist[timer];
    }
    n = timer * 2 + gd->ovcnt - 4;
    if (n < 0) {
        n = 0;
    }
    gd->ovcnt = n;
    if (n < 2) {
        return;
    }
    gd->strcnt = 3;             // cut down stars
    if (n <= 2) {               // overload max
        return;
    }
    // overload. cap ovcnt and wipe out a guy
    gd->ovcnt = 2;
    pobj = (obj_t *)&gd->optr;
    for (;;) {
        obj = pobj->olink;
        if (obj == NULL) {
            return;
        }
        if (obj->otyp == 0) {
            break; // found one!
        }
        pobj = obj;
    }
    pobj->olink = obj->olink;   // unlink

    // hyper him out of there
    obj->ox16 = (seed16() & 0x3fff) + 0x6000 + obj->ox16;
    ofshit(obj);
    obj->objx = obj->objy = 0;
    obj->olink = gd->iptr;      // into inactive list
    gd->iptr = obj;
}

/*
 * Switch processing
 */
static void swp(void)
{
    uint8_t ptype, pdata;
    procfn_t paddr;

    // bit of a hack for REVERSE
    if (cfg.controller == CFG_CONTROLLER_JOYSTICK) {
        if (((gd->joy1 & JOY_RIGHT) && gd->pladir < 0) ||
            ((gd->joy1 & JOY_LEFT) && gd->pladir >= 0)) {
            gd->nplad = -gd->pladir;
        }
    } else if (cfg.controller == CFG_CONTROLLER_JOYPAD) {
        if (((gd->jpad & JOYPAD_RIGHT_BIT) && gd->pladir < 0) ||
            ((gd->jpad & JOYPAD_LEFT_BIT) && gd->pladir >= 0)) {
            gd->nplad = -gd->pladir;
        }
    }

    for (;;) {
        paddr = swproc[0].paddr;
        if (paddr != NULL) {
            ptype = swproc[0].ptype;
            pdata = swproc[0].pdata;
            swproc[0].paddr = NULL;
        } else {
            paddr = swproc[1].paddr;
            if (paddr == NULL) {
                break;
            }
            ptype = swproc[1].ptype;
            pdata = swproc[1].pdata;
            swproc[1].paddr = NULL;
        }
        if ((pdata & gd->status) == 0) {
            mkproc(paddr, ptype);
        }
    }
    if (gd->killme) {
        gd->killme = 0;
        gd->plrx->plas = 0;
        mkproc(plend, STYPE);
    }
}

void exec(void)
{
    if (gd->want_timing) {
        setcolor(0);
    }
    gd->crproc = (proc_t *)&gd->active;
    proc_timer();
    colchk();               // collision check
    expu();                 // explosion update
    rand();                 // update randoms
    swp();                  // switch processing
    if (gd->want_timing) {
        eorcolor(0x007);
    }
    disp();                 // dispatch procs
    if (gd->want_timing) {
        eorcolor(0x077);
    }
}

