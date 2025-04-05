/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Attract mode: Hall of Fame.
 */

#include "gd.h"
#include "screen.h"
#include "data.h"
#include "sprites.h"
#include "color.h"
#include "tdisp.h"
#include "irq_thread.h"
#include "plstrt.h"
#include "stars.h"
#include "setup.h"
#include "mess.h"
#include "sound.h"
#include "cmos.h"
#include "attract.h"

void scores(void)
{
    uint8_t pid;

    pid = gd->plrcnt;
    while (pid != 0) {
        scrtr0(pid);
        --pid;
    }
}

static uint32_t hofscore(const hofent_t *ep)
{
    return (ep->score[0] << 16) | (ep->score[1] << 8) | ep->score[2];
}

void haldt(const hofent_t *hof, uint16_t x, uint8_t y)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        messf(x, y, ID_HALDT_ENTRY,
            i + 1,
            hof[i].name[0],
            hof[i].name[1],
            hof[i].name[2],
            hofscore(&hof[i])
            );
        y += 10;
    }
}

static void hald3(void) __dead;

static void hald3(void)
{
    if (--gd->stalt != 0) {
        sleep(hald3, 10);
    }
    ledret();
}

void haldis(void)
{
    uint16_t *wp;

    gd->hsrflg = 0;
    gncide();
    scrclr();
    pcram[1] = 0;
    mkproc(creds, AMTYPE);
    scores();
    messf(0x38 << 1, 0x54, ID_HALL_OF_FAME);
    messf(0x22 << 1, 0x68, ID_TODAYS);
    messf((0x22 + 0x3e) << 1, 0x68, ID_ALL_TIME);
    messf((0x22 - 4) << 1, 0x68 + 10, ID_GREATEST);
    messf((0x22 + 0x3d) << 1, 0x68 + 10, ID_GREATEST);
    // underline headings
    wp = SCRPTR((0x22 - 4) << 1, 0x7b);
    wp[0] = 0x000f;  wp[80] = 0x000f;
    wp[4] = 0xffff;  wp[84] = 0xffff;
    wp[8] = 0xffff;  wp[88] = 0xffff;
    wp[12] = 0xffff; wp[92] = 0xffff;
    wp[16] = 0xffc0; wp[96] = 0xffc0;

    wp = SCRPTR((0x22 + 0x3d) << 1, 0x7b);
    wp[0] = 0x0003;  wp[80] = 0x0003;
    wp[4] = 0xffff;  wp[84] = 0xffff;
    wp[8] = 0xffff;  wp[88] = 0xffff;
    wp[12] = 0xffff; wp[92] = 0xffff;
    wp[16] = 0xfff0; wp[96] = 0xfff0;

    haldt(todays_hst, 0x18 << 1, 0x86);
    haldt(alltime_hst, 0x59 << 1, 0x86);
    pcram[12] = 0x3f;
    defdraw(0x30 << 1, 0x38);
    mkproc(colr, AMTYPE);
    gd->stalt = 50;
    hald3();
}

static void hofin(void)
{
    blkclr(0x46 << 1, 0xac, 0x14 << 1, 0x08);
    messf(0x46 << 1, 0xac, ID_HOFIN_FMT,
            gd->inits[0], gd->inits[1], gd->inits[2]);
}

static void hoful(void)
{
    uint8_t i, y, c;
    uint16_t sx, x;
    uint32_t *wp;

    sx = 0x46 << 1;
    y = 0xb7;
    blkclr(sx, y, 48, 2);
    for (i = 0; i < 3; i++) {
        c = (i == gd->initn) ? 13 : 1;
        for (x = sx; x < sx + 8; x++) {
            wp = (uint32_t *)SCRPTR(x, y);
            wp[0] |= col_to_planes[x & 0xf][c][0];
            wp[1] |= col_to_planes[x & 0xf][c][1];
            wp[40] |= col_to_planes[x & 0xf][c][0];
            wp[41] |= col_to_planes[x & 0xf][c][1];
        }
        sx += 16;
    }
}

static void hofst(void)
{
    --gd->stalt;
    sleep(hofst, 50);
}

static void hofbl(void)
{
    pcram[13] = (pcram[13] == 0) ? pcram[1] : 0;
    sleep(hofbl, 15 * 5 / 6);
}

static void hofud1(void)
{
    int8_t a;
    char c;

    if (gd->pia21 & PIA21_DOWN) {
        a = -1;
    } else if (gd->pia31 & PIA31_UP) {
        a = 1;
    } else {
        gd->inidir = 0;
        sleep(hofud1, 1);
    }
    if (a == gd->inidir) {
        if (--gd->udcnt == 0) {
            c = gd->inits[gd->initn];
            c += a;
            switch (c) {
            case 'Z' + 1: c = ' '; break;
            case 'A' - 1: c = ' '; break;
            case ' ' + 1: c = 'A'; break;
            case ' ' - 1: c = 'Z'; break;
            default: break;
            }
            gd->inits[gd->initn] = c;
            hofin();
            gd->uddel = (gd->uddel >> 1) + 5;
            gd->udcnt = gd->uddel;
        }
        sleep(hofud1, 1);
    }
    // new direction
    gd->inidir = a;
    gd->uddel = 55 * 5 / 6;
    gd->udcnt = 3;
    sleep(hofud1, 1);
}

static void hofud(void)
{
    gd->inidir = 0;
    hofud1();
}

static void hall6(void) __dead;
static void hall3a(void) __dead;

static void hofas(hofent_t *hof, uint32_t score)
{
    int8_t i;

    for (i = 6; i >= 0; i--) {
        if (hofscore(&hof[i]) < score) {
            hof[i + 1] = hof[i];
        } else {
            break;
        }
    }
    hof[i + 1].name[0] = gd->inits[0];
    hof[i + 1].name[1] = gd->inits[1];
    hof[i + 1].name[2] = gd->inits[2];
    hof[i + 1].score[0] = score >> 16;
    hof[i + 1].score[1] = score >> 8;
    hof[i + 1].score[2] = score;
}

static void save_hst(void)
{
    uint8_t flags;

    flags = irq_thread_disable();
    file_wrapper(NULL, write_hst);
    irq_thread_restore(flags);
}

static void hall1(void) __dead;
static void hall12(void) __dead;

static void hall12(void)
{
    if (++gd->pnumb != 3) {
        gd->pscore = pldata[1].pscor;
        hall1();
    }
    if (gd->alltime_dirty & cfg.autosave_hst) {
        save_hst();
        gd->alltime_dirty = 0;
    }
    if (!gd->entflg) {
        sleep(haldis, 0xff * 5 / 6);
    }
    haldis();
}

static void hall6(void)
{
    gncide();
    hofas(todays_hst, gd->pscore);
    if (gd->pscore > hofscore(&alltime_hst[7])) {
        hofas(alltime_hst, gd->pscore);
        gd->alltime_dirty = 1;
    }
    hall12();
}

static void hall4(void)
{
    if ((gd->pia21 & PIA21_FIRE) == 0) {
        if (gd->stalt == 0) {
            hall6();
        }
        if (++gd->fircnt == 5) {
            gd->firflg = gd->fircnt;
        }
        sleep(hall4, 1);
    }
    gd->fircnt = 0;
    if (gd->firflg) {
        gd->stalt = 20 * 5 / 6;
        ++gd->initn;
        hoful();
        if (gd->initn == 3) {
            hall6();
        }
        gd->fircnt = 0;
        gd->firflg = 0;
    }
    sleep(hall4, 1);
}

static void hall3a(void)
{
    gd->fircnt = 0;
    gd->firflg = 0;
    sleep(hall4, 1);
}

static void hall1(void)
{
    uint8_t snd;

    if (gd->pscore <= hofscore(&todays_hst[7])) {
        hall12();
    }
    ++gd->entflg;
    scrclr();
    pcram[1] = 0x85;
    snd = 0x20; // high score sound phantom (game 0x3e)
    if (gd->pscore > hofscore(&todays_hst[0])) {
        // todays top score
        snd = 0x21; // sound toccata (game 0x3d)
    }
    // game: sound 0x24, select organ - not required
    sndout(snd);
    messf(0x3e << 1, 0x38, ID_PLAYER_S,
        (gd->pnumb == 1) ? "ONE" : "TWO"
        );
    messf(0x14 << 1, 0x58,
        ID_YOU_HAVE_QUALIFIED
        );
    gd->inits[0] = 'A';
    gd->inits[1] = ' ';
    gd->inits[2] = ' ';
    hofin();
    gd->stalt = 40 * 5 / 6;
    mkproc(hofst, AMTYPE);
    mkproc(hofbl, AMTYPE);
    mkproc(hofud, AMTYPE);
    gd->initn = 0;
    hoful();
    hall3a();
}

void hallof(void)
{
    gncide();
    gd->status = 0xff;
    stinit();
    gd->entflg = 0;
    mkproc(creds, AMTYPE);
    gd->pnumb = 1;
    gd->pscore = pldata[0].pscor;
    hall1();
}

