/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Raster IRQ 'C' handler.
 */

#include "gd.h"
#include "mfp.h"
#include "screen.h"
#include "timers.h"

void raster_irq_handler(void)
{
    gd->curirq = 0x44;
}

