/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Interrupt setup.
 */

#include "gd.h"
#include "mfp.h"
#include "irq_thread.h"
#include "sysvars.h"
#include "sys.h"
#include "vectors.h"
#include "timers.h"
#include "irq.h"

static void icache_flush(void)
{
	__asm__ __volatile__(
		"\t.dc.w	0x4e7a,0x0002\n"	// movec	%cacr,%d0
		"\tor.l		#0x008,%%d0\n"		// Clear I-Cache
		"\t.dc.w	0x4e7b,0x0002\n"	// movec	%d0,%cacr
	:
	:
	: "d0", "cc", "memory");
}

static void patch_bra(void *from, void *to)
{
    short diff = (long)to - (long)from - 2;
    ((short *)from)[0] = 0x6000;     // BRA
    ((short *)from)[1] = diff;
}

// configure interrupts for game mode
void irq_init(void)
{
    irq_disable();
    timers_reset_all();
    sys_set_error_vectors_game();
    vector_set(V_VBI, hwvbi);
    if (gd->idata.vid == IDATA_VID_FALCON) {
        vector_set(V_TIMER_B, raster_falcon);
    }
    if (_longframe) {
        patch_bra(irq_thread_restore, irq_thread_restore_long);
        patch_bra(invoke_irq_thread, invoke_irq_thread_long);
        if (gd->idata.cpu == IDATA_CPU_68030) {
            icache_flush();
        }
    } else {
        vector_set(V_TIMER_B, raster);
    }
    mfp_iera_set(1 << 0);
    mfp_imra_set(1 << 0);
}

