/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Thrust image pattern generation.
 */

#include "gd.h"
#include "data.h"
#include "rand.h"
#include "thrust.h"

static uint8_t thtab[64];          // thrust pattern table
static uint8_t *thx;               // thrust table pointer

/*
 * Initialize the thrust plume
 * random pattern table.
 */
void thinit(void)
{
    uint8_t i, rnd;

    thx = thtab;
    for (i = 0; i < 32; i++) {
        do {
            rnd = rand();
        } while (((rnd & 0xf0) == 0) ||
                 ((rnd & 0xf0) == 0xb0) ||
                 ((rnd & 0x0f) == 0) ||
                 ((rnd & 0x0f) == 0xb));
        thtab[i] = rnd;
        thtab[32 + i] = rnd;
    }
}

void thproc(void)
{
    // cycle thx from &thtab[0] to &thtab[32]
    if (++thx == &thtab[33]) {
        thx = &thtab[0];
    }
}

/*
 * Plot the thrust plume when PLADIR +ve (facing right)
 *    +0 +1 +2 +3
 *    ..|..|..|..|OOOOOOOOOOOOOOOO
 * +1 ..|..|..|00|OOOOOOOOOOOOOOOO
 * +2 ..|03|02|01|OOOOOOOOOOOOOOOO
 * +3 07|06|05|04|OOOOOOOOOOOOOOOO
 * +4 ..|0a|09|08|OOOOOOOOOOOOOOOO
 * +5 ..|..|..|0b|OOOOOOOOOOOOOOOO
 */
void thout(pict_t *pict, int thrust)
{
    uint8_t *wp = (uint8_t *)pict->pat;

    if (thrust) {
        wp[3]  = thx[7];  // (0,3)
        wp[8]  = thx[3];  // (1,2)
        wp[9]  = thx[6];  // (1,3)
        wp[10] = thx[10]; // (1,4)
        wp[14] = thx[2];  // (2,2)
        wp[15] = thx[5];  // (2,3)
        wp[16] = thx[9];  // (2,4)
    }
    wp[19] = thx[0];  // (3,1)
    wp[20] = thx[1];  // (3,2)
    wp[21] = thx[4];  // (3,3)
    wp[22] = thx[8];  // (3,4)
    wp[23] = thx[11]; // (3,5)
}

/*
 * Plot the thrust plume when PLADIR -ve (facing left)
 *                  +8 +9 +A +B
 * OOOOOOOOOOOOOOOO|..|..|..|..
 * OOOOOOOOOOOOOOOO|00|..|..|.. +1
 * OOOOOOOOOOOOOOOO|01|02|03|.. +2
 * OOOOOOOOOOOOOOOO|04|05|06|07 +3
 * OOOOOOOOOOOOOOOO|08|09|0a|.. +4
 * OOOOOOOOOOOOOOOO|0b|..|..|.. +5
 */
void thout1(pict_t *pict, int thrust)
{
    uint8_t *wp = (uint8_t *)pict->pat;
    int x, y;

    wp[49] = thx[0];  // (8,1)
    wp[50] = thx[1];  // (8,2)
    wp[51] = thx[4];  // (8,3)
    wp[52] = thx[8];  // (8,4)
    wp[53] = thx[11]; // (8,5)
    if (thrust) {
        wp[56] = thx[2];  // (9,2)
        wp[57] = thx[5];  // (9,3)
        wp[58] = thx[9];  // (9,4)
        wp[62] = thx[3];  // (10,2)
        wp[63] = thx[6];  // (10,3)
        wp[64] = thx[10]; // (10,4)
        wp[69] = thx[7];  // (11,3)
    }
    // shift thrust one pixel left
    for (y = 0; y < 6; y++) {
        x = 7;
        wp[x * 6 + y] |= wp[(x + 1) * 6 + y] >> 4;
        for (x = 8; x < 11; x++) {
            wp[x * 6 + y] <<= 4;
            wp[x * 6 + y] |= wp[(x + 1) * 6 + y] >> 4;
        }
        wp[11 * 6 + y] <<= 4;
    }
}

