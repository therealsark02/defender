/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SPRPAT_H_
#define _SPRPAT_H_

struct sprpat_s;
typedef struct sprpat_s sprpat_t;

struct sprpat_s {
    void (*draw_left)(uint16_t *dst, const uint16_t *src);
    void (*draw_full)(uint16_t *dst, const uint16_t *src);
    void (*draw_right)(uint16_t *dst, const uint16_t *src);
    void (*erase_left)(uint16_t *dst, const uint16_t *src);
    void (*erase_full)(uint16_t *dst, const uint16_t *src);
    void (*erase_right)(uint16_t *dst, const uint16_t *src);
    uint8_t w;                  // visible width in pixels (NOT ncols*16)
    uint8_t h;                  // height
    uint8_t ncols;              // number of 16-pixel columns
    uint16_t *pat[16];          // 16 shifted patterns
};

#endif

