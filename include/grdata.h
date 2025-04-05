/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _GRDATA_H_
#define _GRDATA_H_

#define GROUND_HEIGHT   75
#define GROUND_WIDTH    2048
#define GROUND_COLS     (GROUND_WIDTH / 16)

// grdata.S
extern const uint16_t grdata[GROUND_HEIGHT][GROUND_COLS];

#endif

