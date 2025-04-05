/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Early IRQ code: While the main code is creating sprites, decompessing
 * sounds, and creating other data, this code sets up the overscan raster
 * interrupt and vertical blank interrupt which runs the attract mode code
 * to give the player something interesting to look at.
 */

#include "gd.h"
#include "screen.h"
#include "vectors.h"
#include "color.h"
#include "rand.h"
#include "irq.h"
#include "sys.h"
#include "swtch.h"
#include "timers.h"
#include "attract.h"

typedef intptr_t jmp_buf[13];

static jmp_buf early_irq_jmpbuf;

static void early_vbi_handler(void) __attribute__((used));

// [0:pc, 4:buf]
int setjmp(intptr_t *buf);
asm(
    "setjmp:"
    "   move.l  4(%sp),%a0\n"
    "   move.l  0(%sp),(%a0)\n"
    "   movem.l %d2-%d7/%a2-%a7,4(%a0)\n"
    "   moveq #0,%d0\n"
    "   rts\n");

// [0:pc, 4:buf, 8:val]
void longjmp(intptr_t *buf, int val) __dead;
asm(
    "longjmp:\n"
    "   move.l  4(%sp),%a0\n"
    "   move.l  8(%sp),%d0\n"
    "   movem.l 4(%a0),%d2-%d7/%a2-%a7\n"
    "   move.l  0(%a0),0(%sp)\n"
    "   rts\n");

static int early_irq_exec(void)
{
    proc_t *p;

    rand();
    p = gd->active;
    while (p != NULL) {
        if (--p->ptime == 0) {
            if (setjmp(early_irq_jmpbuf) == 0) {
                gd->crproc = p;
                p->paddr();
                panic(0x43b68dd);
            }
            p = *gd->evbi_pplink;
        } else {
            p = p->plink;
        }
    }
    return (gd->active != NULL);
}

void early_irq_sleep(procfn_t fn, uint8_t ptime)
{
    proc_t *p = gd->crproc;

    p->paddr = fn;
    p->ptime = ptime;
    gd->evbi_pplink = &p->plink;
    longjmp(early_irq_jmpbuf, 1);
}

void early_irq_sucide(void)
{
    proc_t *p = kill(gd->crproc);
    gd->evbi_pplink = &p->plink;
    longjmp(early_irq_jmpbuf, 1);
}

static void early_irq_shutdown(void)
{
    // stop timer B
    writeb(MFP_TCRB, 0);
    gd->evbi_finished = 1;
}

static void early_vbi_handler(void)
{
    if (gd->evbi_finished) {
        return;
    }
    timerb_setup(SCREEN_HALF_Y);
    if (gd->irq_nest) {
        return;
    }
    ++gd->irq_nest;
    if (early_irq_exec() == 0) {
        early_irq_shutdown();
    }
    colr_apply();
    --gd->irq_nest;
}

void early_vbi(void);
asm(
    "early_vbi:"
    "   movem.l %d0-%d1/%a0-%a1/%a6,-(%sp)\n"
    "   move.w  #0x2300,%sr\n"
    "   move.l  #g_gd,%a6\n"
    "   bsr     early_vbi_handler\n"
    "   jsr     sys_vbi\n"
    "   movem.l (%sp)+,%d0-%d1/%a0-%a1/%a6\n"
    "   rte");

void early_irq_init(void)
{
    irq_disable();

    gd->irq_thread_state = 0x7;       // irq thread disabled

    // disable all MFP interrupts
    writeb(MFP_IERA, 0);
    writeb(MFP_IERB, 0);
    writeb(MFP_IMRA, 0);
    writeb(MFP_IMRB, 0);

    // vector VBI
    vector_set(V_VBI, early_vbi);

    // let the OS run for another 5s
    gd->os_enabled = 1;
    gd->os_disable_timer = 250;

    // setup (but not start) timer b
    vector_set(V_TIMER_B, raster);
    mfp_iera_set(1 << 0);
    mfp_imra_set(1 << 0);

    pinit();
    mkproc(ramtest, ATYPE);

    // wait for vblank before enabling ints
    do {
    } while(vidoffs_get() != 0);
    irq_enable();
}

