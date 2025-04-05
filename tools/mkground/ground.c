/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "dtls.h"

bitmap_t *create_ground_bitmap(void)
{
    int ix, iy, bi, dy;
    int miny, maxy;
    bitmap_t *bm;
    int sr, sc;
    int pass;

    bm = calloc(1, sizeof(*bm));
    assert(bm != NULL);

    dy = 0;
    miny = 0xff;
    maxy = 0;
    for (pass = 1; pass <= 2; pass++) {
        bi = 0;
        sr = tdata[bi];
        sc = 7;
        bi = (bi + 1) % tdatasz;
        iy = dy;
        for (ix = 0; ix < 2048; ix++) {
            if (pass == 1) {
                miny = (iy < miny) ? iy : miny;
                maxy = (iy > maxy) ? iy : maxy;
            } else {
                assert(iy >= 0);
                assert(iy < bm->h);
                bm->data[iy * bm->w / 8 + ix / 8] |=
                    1 << (7 - (ix & 7));
            }
            iy = iy + ((sr & 0x80) ? -1 : 1);
            sr <<= 1;
            if (sc-- == 0) {
                sr = tdata[bi];
                bi = (bi + 1) % tdatasz;
                sc = 7;
            }
        }
        if (pass == 1) {
            bm->w = 2048;
            bm->h = maxy - miny + 1;
            bm->data = calloc(1, bm->w * bm->h / 8);
            assert(bm->data != NULL);
            dy = -miny;
        }
    }
    return bm;
}

#define MTIMGH  12
#define MTIMGW  256
uint8_t bmtimg[MTIMGH][MTIMGW / 8];

static void bmtplot_ab8(int a, uint8_t b, uint8_t data)
{
    int x = a << 1;
    assert(b < MTIMGH);
    assert(x < MTIMGW);
    assert(data == 0x00 || data == 0x70 || data == 0x07 || data == 0x77);
    assert(((x + 1) >> 3) < MTIMGW);
    bmtimg[b][(x + 0) >> 3] |= ((data >> 4) ? 1 : 0) << (7 - ((x + 0) & 7));
    bmtimg[b][(x + 1) >> 3] |= ((data & 0xf) ? 1 : 0) << (7 - ((x + 1) & 7));
}

bitmap_t *create_mterr_bitmap(void)
{
    const uint8_t *ep = mterr + mterrsz;
    uint8_t b, xh, xl, minb, maxb;
    const uint8_t *rp;
    bitmap_t *bm;
    size_t bmsz;
    int pass, a;

    bm = calloc(1, sizeof(*bm));
    assert(bm != NULL);

    memset(bmtimg, 0, sizeof(bmtimg));
    minb = 0xff;
    maxb = 0;
    for (pass = 1; pass <= 2; pass++) {
        for (a = 0, rp = mterr; rp != ep; a++, rp += 3) {
            b = rp[0]; xh = rp[1]; xl = rp[2];
            if (pass == 1) {
                minb = (b < minb) ? b : minb;
                maxb = (b > maxb) ? b : maxb;
            } else {
                bmtplot_ab8(a, b - minb + 0, xh);
                bmtplot_ab8(a, b - minb + 1, xl);
            }
        }
    }
    bm->w = MTIMGW;
    bm->h = MTIMGH;
    bmsz = bm->w * bm->h / 8;
    assert(bmsz == sizeof(bmtimg));
    bm->data = calloc(1, bmsz);
    assert(bm->data != NULL);
    memcpy(bm->data, bmtimg, bmsz);
    return bm;
}
