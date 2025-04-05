/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Main program.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "timers.h"
#include "early_irq.h"
#include "irq.h"
#include "vectors.h"
#include "sprites.h"
#include "mkmterr.h"
#include "stars.h"
#include "blips.h"
#include "plend.h"
#include "font.h"
#include "cmos.h"
#include "expl.h"
#include "ground.h"
#include "bgalt.h"
#include "sinit.h"

int main(void)
{
    gd->startup = 1;
    gd->font = &defender_font;
    gd->has_joypad = ((gd->idata.features & IDATA_FEAT_ENHJOY) != 0);
    scrclr();
    gd->pwrflg = 1;

    // build screen y pointer table and color maps
    screen_init();
    crinit_maps();

    // create the temporary raster and vbi environment
    early_irq_init();

    // make chars first, for the attract screen
    mkchars();

    // expand the big data blocks so their data
    // will be added to the malloc pool.
    ground_init();
    sound_init();

    // set up the color map before expanding sprites
    gd->hseed = 0xa5;
    gd->lseed = 0x5a;

    // make data
    mksprites();
    mkmterr();
    build_bgalt();
    stars_init();
    blips_init();
    expl_init();
    plend_init();
    laser_init();
    cminit();

    // ready to go; wait for vbi
    // sequence to end
    gd->emain_finished = 1;
    while (!gd->evbi_finished) {
        asm volatile("" ::: "memory");
    }
    irq_init();
    sound_irq_init();
    keyboard_init();
    sinit();
    return *(int *)0x100;
}
