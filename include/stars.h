/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _STARS_H_
#define _STARS_H_

typedef struct {
    uint8_t sx;             // xpos
    uint8_t sy;             // ypos
    uint8_t scol;           // color
    uint32_t *eradst;
    uint32_t eramask;
} star_t;

#define SNUM    16
extern star_t smap[SNUM];

void stinit(void);
void stars_init(void);

void stout(void);

#endif

