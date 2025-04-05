/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _RAND_H_
#define _RAND_H_

static inline uint16_t seed16(void)
{
    return (gd->seed << 8) | gd->hseed;
}

static inline uint16_t hseed16(void)
{
    return (gd->hseed << 8) | gd->lseed;
}

uint8_t rand(void);
uint8_t randrng(uint8_t lo, uint8_t hi);
uint8_t rmax(uint8_t hi);

#endif

