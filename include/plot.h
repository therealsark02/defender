/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _PLOT_H_
#define _PLOT_H_

#define PLOT_LFR_GENERATOR(a, mac) \
    mac(a, left) \
    mac(a, full) \
    mac(a, right)

#define PLOT_HEIGHT_GENERATOR(mac) \
    mac(1) \
    mac(3) \
    mac(4) \
    mac(6) \
    mac(8)

#define PLOT_LFR_GEN(a, b) \
    void plot_1x##a##_##b(uint16_t *dst, const uint16_t *src); \
    void erase_1x##a##_##b(uint16_t *dst, const uint16_t *src);

#define PLOT_DIM_GEN(a) \
    PLOT_LFR_GENERATOR(a, PLOT_LFR_GEN)

#ifndef __ASSEMBLER__
PLOT_HEIGHT_GENERATOR(PLOT_DIM_GEN)

void plot_2x6_full(uint16_t *dst, const uint16_t *src);
void erase_2x6_full(uint16_t *dst, const uint16_t *src);
void plot_128_12_1bpp(uint16_t *dst, const uint16_t *src);

int spr_collide_full(const uint16_t *src1, const uint16_t *src2, int nrows);
int spr_collide_half(const uint16_t *src1, const uint16_t *src2, int nrows);
#endif

#endif

