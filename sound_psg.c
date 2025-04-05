/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * ST YM2149 sound driver.
 */

#include "gd.h"
#include "ym2149.h"
#include "timers.h"
#include "irq.h"
#include "vectors.h"
#include "sys.h"
#include "tunes.h"
#include "sfxdata.h"
#include "sound_drv.h"

const uint8_t * volatile t_sptr;    // sample pointer
const uint8_t *t_rptr;              // restart pointer
const uint8_t *t_eptr;              // end pointer

void sfxplay1(void);    // play 1 channel, 1 nibble per byte
void sfxplay1p(void);   // play 1 channel, 2 nibbles per byte
void sfxplay2(void);    // play 2 channel, 1 nibble per byte
void sfxplay2p(void);   // play 2 channel, 2 nibbles per byte
void sfxplay3(void);    // play 3 channel, 1 nibble per byte
void sfxplay3p(void);   // play 3 channel, 2 nibbles per byte

// sfx play function [nchans-1][packed]
static const sfxfunc_t sfx_functab[3][2] = {
    [0][0] = sfxplay1,
    [0][1] = sfxplay1p,
    [1][0] = sfxplay2,
    [1][1] = sfxplay2p,
    [2][0] = sfxplay3,
    [2][1] = sfxplay3p,
};

static void sndout_psg(uint8_t cmd)
{
    const uint8_t *o_sptr;
    uint16_t sr;
    sfx_t *sfx;

    psgtimer_stop();
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
        o_sptr = t_sptr;
        if (o_sptr > sfx->max_sptr) {
            sfx->max_sptr = o_sptr;
        }
    }
    gd->cursnd = NULL;

    if (cmd >= SFX_NSOUND_IDS) {
        panic(0x4ffed953, cmd);
    }
    sfx = sfxtab[cmd];
    if (sfx != NULL && sfx->func != NULL) {
        sr = irq_disable();
        t_sptr = sfx->sptr;
        t_eptr = sfx->eptr;
        t_rptr = sfx->rptr;
        vector_set(V_PSG_TIMER, sfx->func);
        psgtimer_setup(MFP_TMR_DIV_4, sfx->tcnt);
        gd->cursnd = sfx;
        irq_restore(sr);
    }
}

static void set_repeat_psg(sfx_t *sfx, uint8_t nchans, uint8_t packed)
{
    uint8_t nvola, nvolb, nvolc;
    uint8_t vola, volb, volc;
    uint32_t len;
    uint8_t m;

    len = (sfx->eptr - sfx->sptr);
    sfx->rptr = sfx->sptr;
    if (packed) {
        switch (nchans) {
        case 1:
            // aa,aa,aa,aa,aa,aa
            vola = *(sfx->eptr - 1) & 0xf;
            for (;;) {
                nvola = *(sfx->rptr) >> 4;
                if (nvola == vola) {
                    break;
                }
                if (++sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b3);
                }
            }
            break;
        case 2:
            // ab,ab,ab,ab,ab,ab
            vola = *(sfx->eptr - 1) >> 4;
            volb = *(sfx->eptr - 1) & 0xf;
            for (;;) {
                nvola = *(sfx->rptr) >> 4;
                nvolb = *(sfx->rptr) & 0xf;
                if (nvola == vola && nvolb == volb) {
                    break;
                }
                if (++sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b4);
                }
            }
            break;
        case 3:
            // ab,ca,bc
            m = len % 3;
            if (m != 0) {
                sfx->eptr -= (3 - m);
            }
            vola = *(sfx->eptr - 2) & 0xf;
            volb = *(sfx->eptr - 1) >> 4;
            volc = *(sfx->eptr - 1) & 0xf;
            for (;;) {
                nvola = *(sfx->rptr + 0) >> 4;
                nvolb = *(sfx->rptr + 0) & 0xf;
                nvolc = *(sfx->rptr + 1) >> 4;
                if (nvola == vola && nvolb == volb && nvolc == volc) {
                    break;
                }
                sfx->rptr += 3;
                if (sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b5);
                }
            }
            break;
        }
    } else {
        switch (nchans) {
        case 1:
            // 0a,0a,0a,0a,0a,0a
            vola = *(sfx->eptr - 1);
            for (;;) {
                nvola = *sfx->rptr;
                if (nvola == vola) {
                    break;
                }
                if (++sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b2);
                }
            }
            break;
        case 2:
            // 0a,0b,0a,0b,0a,0b
            m = len % 2;
            if (m != 0) {
                sfx->eptr -= (2 - m);
            }
            vola = *(sfx->eptr - 2);
            volb = *(sfx->eptr - 1);
            for (;;) {
                nvola = sfx->rptr[0];
                nvolb = sfx->rptr[1];
                if (nvola == vola && nvolb == volb) {
                    break;
                }
                sfx->rptr += 2;
                if (sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b1);
                }
            }
            break;
        case 3:
            // 0a,0b,0c,0a,0b,0c
            m = len % 3;
            if (m != 0) {
                sfx->eptr -= (3 - m);
            }
            vola = *(sfx->eptr - 3);
            volb = *(sfx->eptr - 2);
            volc = *(sfx->eptr - 1);
            for (;;) {
                nvola = sfx->rptr[0];
                nvolb = sfx->rptr[1];
                nvolc = sfx->rptr[2];
                if (nvola == vola && nvolb == volb && nvolc == volc) {
                    break;
                }
                sfx->rptr += 3;
                if (sfx->rptr >= sfx->eptr) {
                    panic(0x2fec35b0);
                }
            }
            break;
        }
    }
}

static void sfx_init_psg(sfx_t *sfx, uint16_t freq, uint8_t nchans, uint8_t flags)
{
    sfx->tcnt = 2457600U / 4 / freq;
    if (flags & SFXF_PACKED) {
        sfx->func = sfx_functab[nchans - 1][1];
    } else {
        sfx->func = sfx_functab[nchans - 1][0];
    }
    if (flags & SFXF_REPEAT) {
        set_repeat_psg(sfx, nchans, !!(flags & SFXF_PACKED));
    }
}

static void snd_init_psg(void)
{
    ym_write(YM_ENABLE, 0xff);
}

static void snd_irq_init_psg(void)
{
    psgtimer_stop();
    mfp_ierb_set(1 << MFP_PSGTIMER_BIT);
    mfp_imrb_set(1 << MFP_PSGTIMER_BIT);
}

sound_drv_t psg_snd_drv = {
    .init = snd_init_psg,
    .irq_init = snd_irq_init_psg,
    .sfx_init = sfx_init_psg,
    .snd_out = sndout_psg
};
