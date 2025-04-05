/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _BGALT_H_
#define _BGALT_H_

void build_bgalt(void);
extern uint8_t alttbl[];

// lookup the terrain altitude
// ox16: object x position in s10p6 format
static inline uint8_t getalt(uint16_t ox16)
{
    return alttbl[(ox16 >> 6) & 0x3ff];
}

#endif

