/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Vertical Blank Interrupt 'C' code.
 */

#include "gd.h"
#include "screen.h"
#include "color.h"
#include "timers.h"
#include "tunes.h"
#include "sys.h"
#include "vbi.h"

void vbi_irq_handler(void)
{
    gd->curirq = 0x99;
    ++gd->frame;
    timerb_setup(SCREEN_HALF_Y);
    tunes_timer();
    sys_vbi();
}

