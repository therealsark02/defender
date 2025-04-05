/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _IRQ_THREAD_H_
#define _IRQ_THREAD_H_

void irq_thread(void);

void irq_thread_restore(uint8_t flag);
uint8_t irq_thread_disable(void);

static inline void irq_thread_enable(void)
{
    irq_thread_restore(1);
}

#endif

