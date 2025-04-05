/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _FONT_H_
#define _FONT_H_

#define NFONTCHARS  69

typedef struct font_s {
    const uint8_t *glyphs;
    const uint8_t *widths;
    uint8_t only_upper;
    uint8_t fixed_width;
    uint8_t charsp;
} font_t;

void render_char(uint16_t x, uint8_t y, const uint8_t *gp);

extern const font_t defender_font;
extern const font_t atari_font ;

#endif

