/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * STE DMA sound driver.
 */

#include "gd.h"
#include "irq.h"
#include "tunes.h"
#include "sfxdata.h"
#include "sound_dma.h"
#include "sound_drv.h"

static void sndout_dma(uint8_t cmd)
{
    sfx_t *sfx;
    const uint8_t *o_sptr;
    uint16_t sr;

    // sound off
    writeb(SDMA_CTRL, SDMA_CTRL_DISABLE);
    tunes_stop();
    if (cmd == 0x13) {
        return;
    }
    if (gd->snddis) {
        return;
    }
    if (cmd == 0x20 || cmd == 0x21) {
        tunes_start(cmd);
        return;
    }
    sfx = gd->cursnd;
    if (sfx != NULL) {
        // get stats from the old sound
        o_sptr = (const uint8_t *)sdma_counter_get();
        if (o_sptr > sfx->max_sptr) {
            sfx->max_sptr = o_sptr;
        }
    }
    gd->cursnd = NULL;

    if (cmd >= SFX_NSOUND_IDS) {
        panic(0x4faed953, cmd);
    }
    sfx = sfxtab[cmd];
    if (sfx != NULL) {
        sr = irq_disable();
        sdma_start_set((uint32_t)sfx->sptr);
        sdma_end_set((uint32_t)sfx->eptr);
        writew(SDMA_MODE, sfx->mode);
        writeb(SDMA_CTRL, sfx->ctrl);
        gd->cursnd = sfx;
        irq_restore(sr);
    }
}

static void set_repeat_dma(sfx_t *sfx)
{
    uint8_t samp0;

    samp0 = sfx->sptr[0];
    while (sfx->eptr > sfx->sptr) {
        if (sfx->eptr[1] == samp0) {
            return;
        }
        sfx->eptr -= 2;
    }
    panic(0x12fedc64);
}

static void sfx_init_dma(sfx_t *sfx, uint16_t freq, uint8_t nchans, uint8_t flags)
{
    // establish frequency control
    sfx->mode = SDMA_MODE_MONO;
    switch (freq) {
    case 50066: sfx->mode |= SDMA_MODE_50066HZ; break;
    case 25033: sfx->mode |= SDMA_MODE_25033HZ; break;
    case 12517: sfx->mode |= SDMA_MODE_12517HZ; break;
    case 6258:  sfx->mode |= SDMA_MODE_6258HZ; break;
    default:    panic(0xde12657c, freq);
    }

    // make sure sample start/end are word aligned
    if ((uint32_t)sfx->sptr & 1) {
        ++sfx->sptr;
        ++sfx->max_sptr;
    }
    if ((uint32_t)sfx->eptr & 1) {
        --sfx->sptr;
    }
    // repeat or not
    sfx->ctrl = SDMA_CTRL_ENABLE;
    if (flags & SFXF_REPEAT) {
        sfx->ctrl |= SDMA_CTRL_REPEAT;
        set_repeat_dma(sfx);
    }
}

static void snd_no_op(void)
{
}

sound_drv_t dma_snd_drv = {
    .init = snd_no_op,
    .irq_init = snd_no_op,
    .sfx_init = sfx_init_dma,
    .snd_out = sndout_dma
};
