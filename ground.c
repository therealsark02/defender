/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Terrain pattern generation and drawing.
 */

#include "gd.h"
#include "screen.h"
#include "grdata.h"
#include "ground.h"

static uint16_t *gr_ptr[16][GROUND_COLS + SCREEN_COLS - 1];
static uint16_t *gr_eptr[16][GROUND_COLS + SCREEN_COLS - 1];

static void ground_build(int shift)
{
    int col, y, offs, nrows, lcol, rcol, pass;
    uint16_t *wp, *op;
    uint16_t data;
    void **fptr;

    wp = heap;
    for (pass = 1; pass <= 2; pass++) {
        for (col = 0; col < GROUND_COLS + SCREEN_COLS - 1; col++) {
            if (pass == 1) {
                gr_ptr[shift][col] = wp;
            } else {
                gr_eptr[shift][col] = wp;
            }
            lcol = col % GROUND_COLS;
            rcol = (col + 1) % GROUND_COLS;
            for (y = 0; y < GROUND_HEIGHT; y++) {
                data = (grdata[y][lcol] << shift) |
                       (grdata[y][rcol] >> (16 - shift));
                if (data != 0) {
                    break;
                }
            }
            op = wp++;
            offs = y;
            fptr = (void **)wp;
            wp += 2;
            nrows = 0;
            for (; y < GROUND_HEIGHT; y++) {
                data = (grdata[y][lcol] << shift) |
                       (grdata[y][rcol] >> (16 - shift));
                if (data == 0) {
                    break;
                }
                if (pass == 1) {
                    *wp++ = data;
                }
                ++nrows;
            }
            if (pass == 1) {
                *(const void **)fptr = ground_ftab[nrows - 1];
            } else {
                *(const void **)fptr = ground_eftab[nrows - 1];
            }
            *op = (offs - (16 - nrows)) * 160;
        }
    }
    heap = wp;
}

void ground_init(void)
{
    int shft;

    for (shft = 0; shft < 16; shft++) {
        ground_build(shft);
    }
    heap_add((void *)grdata, sizeof(grdata));
}

static void ground_draw(int x, int y)
{
    uint16_t *dst = SCRPTR(16, y) + 3;
    int shift, col;

    shift = x & 0xf;
    col = x >> 4;
    plot_ground(dst, gr_ptr[shift][col]);
}

static void ground_erase(int x, int y)
{
    uint16_t *dst = SCRPTR(16, y) + 3;
    int shift, col;

    shift = x & 0xf;
    col = x >> 4;
    erase_ground(dst, gr_eptr[shift][col]);
}

#include "plot.h"
#include "mkmterr.h"

void bgout(void)
{
    int ox = (s10p6_to_screenx(gd->bglx) + 18) & (GROUND_WIDTH - 1);
    int nx = (s10p6_to_screenx(gd->bgl) + 18) & (GROUND_WIDTH - 1);
    ground_erase(ox, GROUND_YPOS);
    ground_draw(nx, GROUND_YPOS);
}

void bgeras(void)
{
    int ox = (s10p6_to_screenx(gd->bglx) + 18) & (GROUND_WIDTH - 1);
    ground_erase(ox, GROUND_YPOS);
}

