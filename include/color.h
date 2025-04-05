/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _COLOR_H_
#define _COLOR_H_

void crinit_maps(void);
void crinit(void);
void colr_apply(void);

extern uint8_t pcram[16];
extern const uint8_t coltab[];
extern uint32_t col_to_planes[16][16][2];   // [shift][col][idx]

void tiecol(void);
void cbomb(void);
void colr(void);

#endif

