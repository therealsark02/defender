/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Late initialization.
 */

#include "gd.h"
#include "screen.h"
#include "plstrt.h"
#include "attract.h"
#include "setup.h"
#include "irq.h"
#include "irq_thread.h"
#include "exec.h"
#include "sinit.h"

void sinit(void)
{
    /*
     * SINIT
     */
    if (!gd->startup) {
        scrclr();
    }
    gd->pminy = 0x70;
    gd->pmaxy = 0xff;
    gd->hseed = 0xa5;
    gd->lseed = 0x5a;
    // set status and irq_thread_disable before the next
    // couple of calls which may invoke irq_thread_disable/restore
    gd->status = 0xff;
    gd->irq_thread_state = 0x5; // disabled,not-pending
    pinit();
    init20();
    if (gd->startup) {
        attr_resume();
    } else {
        mkproc(st1, STYPE);
    }
    // wait for vblank before enabling ints
    do {
    } while(vidoffs_get() != 0);
    irq_enable();
    irq_thread_enable();
    exec();
}
