/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SCREEN_H_
#define _SCREEN_H_

#define SCREEN_WIDTH        320
#define SCREEN_HEIGHT       245

#define SCREEN_COLS (SCREEN_WIDTH / 16)

#define YMAX                240
#define YMIN                42

#define YMAX_u8p8           u8_to_u8p8(YMAX)
#define YMIN_u8p8           u8_to_u8p8(YMIN)

#ifndef __ASSEMBLER__
extern uint16_t *scrptr[256];
void screen_init(void);
#endif

#define YPOS_LOG_TO_PHYS(y)     ((y) - 7)
#define YPOS_PHYS_TO_LOG(y)     ((y) + 7)

// physical screen counter offset
#define VIDOFFS(y)      ((y) * 160)

#define SCREEN_HALF_Y   122     // half frame from vsync

// screen pointer for all display accesses.
// uses scraddr to shift all y positions up 7 rows (y - 7)
#define SCRPTR(x, y)    (scrptr[y] + (((x) >> 2) & 0xfffc))

#ifndef __ASSEMBLER__

static inline uint32_t vidbase_get(void)
{
    uint8_t *p = (uint8_t *)0xffff8201;
    uint16_t val;

    asm volatile("movep.w 0(%1),%0" : "=r"(val) : "a"(p));
    return (uint32_t)val << 8;
}

static inline void vidbase_set(uint32_t val)
{
    uint8_t *p = (uint8_t *)0xffff8201;

    asm volatile("movep.w %0,0(%1)" :: "r"(val >> 8), "a"(p));
}

static inline uint16_t vidoffs_get(void)
{
    volatile uint8_t *p = (uint8_t *)0xffff8203;
    uint32_t val;

    asm volatile("movep.l 0(%1),%0" : "=r"(val) : "a"(p));
    return (val & 0xffffff) - gd->vid_base;
}

// low-res, 50Hz
static inline void vidmode_set(void)
{
    *(uint8_t *)0xffff820a = 0x02;      // 50Hz
    *(uint8_t *)0xffff8260 = 0x00;      // Low res
}

// palette manipulation
static inline void eorcolor(uint16_t n)
{
    *(volatile uint16_t *)0xffff8240 ^= n;
}

static inline void setcolor(uint16_t n)
{
    *(volatile uint16_t *)0xffff8240 = n;
}

// clear the whole screen
static inline void scrclr(void)
{
    memset((char *)gd->vid_base, 0, 160 * SCREEN_HEIGHT);
}

// clear the screen below the scanner
static inline void sclr1(void)
{
    memset(scrptr[YMIN],
        0, 160 * (SCREEN_HEIGHT - YMIN));
}
#endif

#endif

