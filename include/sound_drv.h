/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SOUND_DRV_H_
#define _SOUND_DRV_H_

typedef void (*sfxfunc_t)(void);

typedef struct sfx_s {
    uint8_t sound;              // sound id
    union {
        uint8_t tcnt;           // timer d counter (psg)
        struct {
            uint16_t mode;      // dma mode + frequency
            uint8_t ctrl;       // dma enable + opt repeat
        };
    };
    sfxfunc_t func;             // play function
    const uint8_t *sptr;        // sample start
    const uint8_t *eptr;        // sample end
    const uint8_t *rptr;        // repeat point
    const uint8_t *max_sptr;    // max play observed
} sfx_t;

#define SFX_NSOUND_IDS  0x20    // num sound ids (currently 0x1..0x1f)

extern sfx_t *sfx;              // sounds loaded
extern uint8_t nsfx;            // number of sounds loaded

extern sfx_t *sfxtab[SFX_NSOUND_IDS];  // direct sound_id -> sfx_t

typedef struct {
    void (*init)(void);
    void (*irq_init)(void);
    void (*sfx_init)(sfx_t *sfx, uint16_t freq, uint8_t nchans, uint8_t flags);
    void (*snd_out)(uint8_t cmd);
} sound_drv_t;

extern sound_drv_t psg_snd_drv;
extern sound_drv_t dma_snd_drv;

#endif

