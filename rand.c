/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Random number generation.
 */

#include "gd.h"
#include "rand.h"

uint8_t rand(void)
{
    uint8_t a, b;
    uint16_t tmp;

    b = gd->seed * 3 + 17;
    a = (gd->lseed >> 3) ^ gd->lseed;
    gd->lseed = (gd->hseed << 7) | (gd->lseed >> 1);
    gd->hseed = (a << 7) | (gd->hseed >> 1);
    a >>= 1;
    tmp = b + gd->lseed;
    gd->seed = tmp + gd->hseed + (tmp >> 8);
    return gd->seed;
}

uint8_t randrng(uint8_t lo, uint8_t hi)
{
    uint8_t a;

    do {
        a = rand();
    } while (a < lo || a > hi);
    return a;
}

uint8_t rmax(uint8_t hi)
{
    uint8_t r;

    r = rand();
    while (r > hi) {
        r >>= 1;
    }
    return r + 1;
}
