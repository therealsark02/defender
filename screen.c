/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Screen row-pointers initialization.
 */

#include "gd.h"
#include "screen.h"

uint16_t *scrptr[256];

/*
 * Initialize the screen start address for each Y coordinate.
 * The top of the physical screen is Y coordinate 7.
 * Table entries 0..6 all point to physical row 0.
 */
void screen_init(void)
{
    uint16_t *ptr;
    uint16_t y;

    // create the y-to-address table
    ptr = (uint16_t *)gd->vid_base;
    for (y = 0; y < YPOS_PHYS_TO_LOG(0); y++) {
        scrptr[y] = ptr;
    }
    for (; y < 256; y++) {
        scrptr[y] = ptr;
        ptr += 80;
    }
}
