/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _FMT_H_
#define _FMT_H_

// u8
#define u8_to_screenx(n)    ((n) << 1)
#define screenx_to_u8(n)    ((n) >> 1)

// u8p8 - used for play16 and oy16
#define u8_to_u8p8(n)       ((n) << 8)
#define u8p8_to_u8(n)       ((n) >> 8)
#define u8p8_to_screenx(n)  ((n) >> 7)
#define screenx_to_u8p8(n)  ((n) << 7)

// s8p8
#define s8_to_s8p8(n)       ((n) << 8)
#define s8p8_to_s8(n)       ((n) >> 8)

// s10p6 - used for ox16, oxv, plabx, bgl
#define screenx_to_s10p6(n)     ((n) << 5)
#define s10p6_to_screenx(n)     ((n) >> 5)
#define s10p6_to_u8(n)          ((n) >> 6)
#define s8_to_s10p6(n)          ((n) << 6)

#endif
