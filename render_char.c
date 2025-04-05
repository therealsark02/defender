/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Render a font character on the screen.
 */

#include "gd.h"
#include "screen.h"
#include "font.h"

void render_char(uint16_t x, uint8_t y, const uint8_t *gp)
{
    uint16_t *dst, shift;
    uint32_t ll;
    uint8_t r;

    dst = SCRPTR(x, y);
    shift = x & 0xf;
    for (r = 0; r < 8; r++) {
        ll = gp[r] << (24 - shift);
        dst[0] |= ll >> 16;
        dst[4] |= ll;
        dst += 80;
    }
}

