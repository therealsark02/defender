/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _GROUND_H_
#define _GROUND_H_

#define GROUND_YPOS     ((YMAX - 6) - GROUND_HEIGHT)

extern char ground_start[];
extern const void *ground_ftab[];
extern const void *ground_eftab[];

void plot_ground(void *dst, const void *src);
void erase_ground(void *dst, const void *src);

void ground_init(void);
void bgout(void);
void bgeras(void);

#endif

