/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Sprites builder.
 */

#include "gd.h"
#include "sprites.h"
#include "thrust.h"
#include "fireball.h"
#include "convert.h"

const pict_t **pictab;
sprpat_t *sprtab;
int npictab;

sprpat_t *sprchrtab;

// create player ship patters with:
//  -  2 directions
//  -  2 thrust states (on and off)
//  - 12 random patterns of thrust
static void mkplayer(void)
{
    const pict_t *pl[2];
    int i, d, pi, th;
    pict_t *pict;

    thinit();
    pi = NALLPICTS;
    pl[0] = &plapic;
    pl[1] = &plbpic;
    for (d = 0; d < 2; d++) { // direction left/right
        for (th = 0; th < 2; th++) { // thrust off/on
            for (i = 0; i < 12; i++) { // thrust random pattern
                pict = malloc(sizeof(pict_t));
                pict->w = pl[d]->w + 4;
                pict->h = pl[d]->h;
                pict->pat = zalloc(pict->w * pict->h);
                if (d == 0) {
                    memcpy((uint8_t *)pict->pat + 4 * pict->h,
                            pl[d]->pat, pl[d]->w * pl[d]->h);
                    thout(pict, th);
                } else {
                    memcpy((uint8_t *)pict->pat,
                            pl[d]->pat, pl[d]->w * pl[d]->h);
                    thout1(pict, th);
                }
                thproc();
                pictab[pi++] = pict;
            }
        }
    }
    if (pi != P_FIREBALL) {
        panic(0xde311fed, pi, P_FIREBALL);
    }
}

// create fireball patterns
// - 12 patterns
static void mkfireball(void)
{
    const uint8_t *fbx;
    pict_t *pict;
    uint8_t *wp;
    int i, pi;

    fbinit();
    pi = P_FIREBALL;
    for (i = 0; i < 12; i++) {
        pict = malloc(sizeof(pict_t));
        pict->w = 2;
        pict->h = 3;
        wp = zalloc(pict->w * pict->h);
        fbx = fbget();
        /* 
         *   0 | 1      0 | 1
         * 0 .O|..    0  0|3
         * 1 OO|O.    1  1|4
         * 2 .O|..    2  2|5
         */
        wp[0] = fbx[0] & 0x0f;
        wp[1] = fbx[1] & 0xff;
        wp[2] = fbx[2] & 0x0f;
        wp[3] = 0x00;
        wp[4] = fbx[3] & 0xf0;
        wp[5] = 0x00;
        pict->pat = wp;
        fbproc();
        pictab[pi++] = pict;
    }
}

// create scanner player blip
static void mkplayerblip(void)
{
    pict_t *pict;
    uint8_t *wp;
    int pi;

    pi = P_PLAYERBLIP;
    pict = malloc(sizeof(pict_t));
    pict->w = 2;
    pict->h = 3;
    wp = zalloc(pict->w * pict->h);
    /* 
     *   0 | 1      0 | 1
     * 0 .O|..    0  0|3
     * 1 OO|O.    1  1|4
     * 2 .O|..    2  2|5
     */
    wp[0] = 0x09;
    wp[1] = 0x99;
    wp[2] = 0x09;
    wp[3] = 0x00;
    wp[4] = 0x90;
    wp[5] = 0x00;
    pict->pat = wp;
    pictab[pi++] = pict;

    if (pi != npictab) {
        panic(0xfeba9323, pi, npictab);
    }
}

#pragma GCC optimize("Os")
void mkchars(void)
{
    int i;

    // convert digit characters
    sprchrtab = malloc(sizeof(sprpat_t) * NCHRTBL);
    for (i = 0; i < NCHRTBL; i++) {
        convert_pict(&sprchrtab[i], &chrtbl[i]);
    }
    // indicate characters ready to use
    gd->chars_ready = 1;
}

void mksprites(void)
{
    int i;

    // convert sprites
    npictab = P__NUM;
    if (npictab != NALLPICTS + 61) {
        panic(0xfe53b013, npictab, P__NUM);
    }
    pictab = malloc(sizeof(pict_t *) * npictab);
    sprtab = malloc(sizeof(sprpat_t) * npictab);
    for (i = 0; i < NALLPICTS; i++) {
        pictab[i] = allpicts[i];
    }

    mkplayer();
    mkfireball();
    mkplayerblip();

    // convert all sprite picts 
    for (i = 0; i < npictab; i++) {
        convert_pict(&sprtab[i], pictab[i]);
    }

}

