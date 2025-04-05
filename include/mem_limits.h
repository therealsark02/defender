/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _MEM_LIMITS_H_
#define _MEM_LIMITS_H_

/*
 * Memory used from _start to heap.
 */

// values sampled from the game itself
#define SAMP_MEM_USED_PSG       0x0d24aa
#define SAMP_MEM_USED_DMA       0x16d5d0

// add 2kB, and round up to the next 2kB
#define MEM_USED_ROUNDUP(x)     (((x) + 0x1000) & -0x800)

// values used by the game
#define MEM_USED_PSG            MEM_USED_ROUNDUP(SAMP_MEM_USED_PSG)
#define MEM_USED_DMA            MEM_USED_ROUNDUP(SAMP_MEM_USED_DMA)

#endif

