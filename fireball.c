/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Fireballs are the 2x2 pixel bullets that enemies fire. They cycle
 * between 4 patterns for a twinkle effect.
 */

#include "gd.h"
#include "rand.h"
#include "fireball.h"

static uint8_t fbtab[32];
static uint8_t *fbx;

void fbinit(void)
{
    uint8_t i, b;
    int8_t r;

    fbx = fbtab;
    for (i = 0; i < 32; i++) {
        r = rand();
        b = (r < 0) ? 0x0a : 0x09;
        fbx[i] = b + ((r & 1) ? 0x90 : 0xa0);
    }
}

const uint8_t *fbget(void)
{
    return fbx;
}

void fbproc(void)
{
    fbx = fbx + 1;
    if (fbx == &fbtab[24]) {
        fbx = fbtab;
    }
}

