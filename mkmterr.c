/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Mini-terrain (in the scanner) pattern generation.
 */

#include "gd.h"
#include "mterr.h"
#include "mkmterr.h"

uint16_t *mterrpat[8];

void mkmterr(void)
{
    const uint32_t *rp;
    uint16_t i, ix, iy;
    uint32_t *wp;
    uint32_t *ep;

    mterrpat[0] = mterr;
    wp = malloc(7 * MTERRW * MTERRH / 8);
    ep = (uint32_t *)((char *)wp + (7 * MTERRW * MTERRH / 8));
    for (i = 1; i < 8; i++) {
        mterrpat[i] = (uint16_t *)wp;
        rp = (uint32_t *)mterrpat[i - 1];
        for (iy = 0; iy < MTERRH; iy++) {
            for (ix = 0; ix < (MTERRW / 32); ix++) {
                wp[ix] = (rp[ix] << 2) | (rp[ix+1] >> 30);
            }
            wp += (MTERRW / 32); rp += (MTERRW / 32);
        }
    }
    if (wp != ep) panic(0xdeadead);
}

