/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _IRQ_H_
#define _IRQ_H_

void irq_init(void);

static inline uint16_t irq_disable(void)
{
    uint16_t sr;

    asm volatile("move.w %%sr,%0\n"
                 "move.w #0x2700,%%sr"
                : "=d"(sr));
    return sr;
}

static inline void irq_restore(uint16_t sr)
{
    asm volatile("move.w %0,%%sr" :: "d"(sr));
}

static inline void irq_enable(void)
{
    irq_restore(0x2300);
}

void hwvbi(void);
void hwkeyb(void);
void raster(void);
void raster_falcon(void);
void irq_thread_restore_long(void);
void invoke_irq_thread(void);
void invoke_irq_thread_long(void);

#endif

