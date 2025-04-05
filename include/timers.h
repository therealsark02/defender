/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "mfp.h"

#ifndef __ASSEMBLER__

void timers_reset_all(void);

static inline void timerb_setup(uint8_t cnt)
{
    writeb(MFP_TCRB, 0);
    do {
        writeb(MFP_TDRB, cnt);
    } while (readb(MFP_TDRB) != cnt);
    writeb(MFP_TCRB, 8);
}

static inline void timerc_stop(void)
{
    uint8_t tccd = readb(MFP_TCCD) & 0x0f;

    writeb(MFP_TCCD, tccd);
}

static inline void timerd_stop(void)
{
    uint8_t tccd = readb(MFP_TCCD) & 0xf0;

    writeb(MFP_TCCD, tccd);
}

static inline void timerc_setup(uint8_t div, uint8_t cnt)
{
    uint8_t tccd = readb(MFP_TCCD) & 0x0f;

    writeb(MFP_TCCD, tccd);
    do {
        writeb(MFP_TDRC, cnt);
    } while (readb(MFP_TDRC) != cnt);
    writeb(MFP_TCCD, (div << 4) | tccd);
}

static inline void timerd_setup(uint8_t div, uint8_t cnt)
{
    uint8_t tccd = readb(MFP_TCCD) & 0xf0;

    writeb(MFP_TCCD, tccd);
    do {
        writeb(MFP_TDRD, cnt);
    } while (readb(MFP_TDRD) != cnt);
    writeb(MFP_TCCD, tccd | div);
}

#endif

#define psgtimer_setup      timerd_setup
#define psgtimer_stop       timerd_stop
#define MFP_PSGTIMER_BIT    4
#define V_PSG_TIMER         V_TIMER_D
#define MFP_TCCD_MASK       0xf0

#endif

