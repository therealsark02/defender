/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * HELP screen.
 */

#include "gd.h"
#include "screen.h"
#include "data.h"
#include "sprites.h"
#include "color.h"
#include "obj.h"
#include "tdisp.h"
#include "cmos.h"
#include "plstrt.h"
#include "stars.h"
#include "setup.h"
#include "mess.h"
#include "attract.h"

static void help1(void) __dead;

static void help1(void)
{
    if (--gd->stalt != 0) {
        sleep(help1, 10);
    }
    ledret();
}

static void help(void)
{
    scrclr();
    pcram[1] = 0;
    mkproc(creds, AMTYPE);
    scores();
    messf(160-5*8, 84, ID_CONTROLS);
    messf(16, 104, ID_CONTROLS_TEXT);
    setup_show_keys(gd->curser_y + 10);
    pcram[12] = 0x3f;
    defdraw(0x30 << 1, 0x38);
    plot_textimg(blurbimg_x, 0xc4, blurbimg_nw, blurbimg_h, blurbimg);
    mkproc(colr, AMTYPE);
    gd->stalt = 50;
    help1();
}

void helpsw(void)
{
    if (gd->status & ST_GAMEOV) {
        gncide();
        gd->status = 0xff;
        sleep(help, 1);
    }
    sucide();
}

