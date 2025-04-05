/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Sound init, sequencer, driver dispatch.
 */

#include "gd.h"
#include "swtch.h"
#include "lzg.h"
#include "irq_thread.h"
#include "sfxdata.h"
#include "sound.h"
#include "sound_drv.h"

static sound_drv_t *snd_drv;

/*
 * Sound Table
 * sndpri, n*(repcnt, sndtmr, snd#) n>=1
 * sndtmr in 20 msec (PAL)
 * sndpri : hi=hi, interruptable by equals
 * end: repcnt=0
 */
const uint8_t cnsnd[]  = { 0xff, 0x01, 0x14, 0x19, 0 }; // coin sound
const uint8_t rpsnd[]  = { 0xff, 0x01, 0x1b, 0x1e, 0 }; // free ship
const uint8_t pdsnd[]  = { 0xf0, 0x02, 0x07, 0x11, 0x01, 0x1b, 0x17, 0 }; // player death
const uint8_t st1snd[] = { 0xf0, 0x01, 0x35, 0x0a, 0 }; // start 1
const uint8_t st2snd[] = { 0xf0, 0x01, 0x0d, 0x0b, 0 }; // start2
const uint8_t tbsnd[]  = { 0xe8, 0x01, 0x03, 0x14, 0x02, 0x05, 0x11, 0x02, 0x08, 0x17, 0 }; // terrain blow
const uint8_t sbsnd[]  = { 0xe8, 0x06, 0x03, 0x11, 0x01, 0x0d, 0x17, 00 }; // smart bomb
const uint8_t acsnd[]  = { 0xe0, 0x03, 0x08, 0x08, 0 }; // astro catch
const uint8_t alsnd[]  = { 0xe0, 0x01, 0x14, 0x1f, 0 }; // astro land
const uint8_t ahsnd[]  = { 0xe0, 0x01, 0x14, 0x11, 0 }; // astro hit
const uint8_t ascsnd[] = { 0xd8, 0x01, 0x0d, 0x1a, 0 }; // astro scream
const uint8_t apsnd[]  = { 0xd0, 0x01, 0x28, 0x15, 0 }; // appear sound
const uint8_t prhsnd[] = { 0xd0, 0x01, 0x0d, 0x05, 0 }; // probe hit
const uint8_t schsnd[] = { 0xd0, 0x01, 0x07, 0x17, 0 }; // schitz hit
const uint8_t ufhsnd[] = { 0xd0, 0x01, 0x07, 0x07, 0 }; // ufo hit
const uint8_t tihsnd[] = { 0xd0, 0x01, 0x08, 0x01, 0 }; // tie hit
const uint8_t lhsnd[]  = { 0xd0, 0x01, 0x08, 0x06, 0 }; // lander hit
const uint8_t lpksnd[] = { 0xd0, 0x01, 0x0d, 0x0b, 0 }; // lander pick up
const uint8_t lsksnd[] = { 0xc8, 0x0a, 0x01, 0x0e, 0 }; // lander suck
const uint8_t swhsnd[] = { 0xc0, 0x01, 0x07, 0x07, 0 }; // swarm hit
const uint8_t lassnd[] = { 0xc0, 0x01, 0x28, 0x14, 0 }; // laser
const uint8_t lgsnd[]  = { 0xc0, 0x01, 0x1b, 0x18, 0 }; // lander grab
const uint8_t lshsnd[] = { 0xc0, 0x01, 0x07, 0x03, 0 }; // lander shoot
const uint8_t sshsnd[] = { 0xc0, 0x01, 0x28, 0x09, 0 }; // schitzo shoot
const uint8_t ushsnd[] = { 0xc0, 0x01, 0x07, 0x03, 0 }; // ufo shoot
const uint8_t swssnd[] = { 0xc0, 0x01, 0x14, 0x0c, 0 }; // swarm shoot

sfx_t *sfx;
uint8_t nsfx;
sfx_t *sfxtab[SFX_NSOUND_IDS];

void sndld(const uint8_t *snd)
{
    uint8_t pri;

    gd->thflg = 0;              // reset thrust
    pri = snd[0];
    if (pri >= gd->sndpri) {    // pri >= current?
        gd->sndpri = pri;       // yes
        irq_thread_disable();   // mutex the sequencer
        gd->sndx = snd - 2;     // sequencer will use sndx + 3
        gd->sndtmr = 1;
        gd->sndrep = 1;
        irq_thread_enable();
    }
}

void sndout(uint8_t cmd)
{
    snd_drv->snd_out(cmd);
}

// gd->sndsx ->{[repcnt,sndtmr,snd#]+,0}
static void sound_sequencer(void)
{
    if (gd->sndtmr != 0) {              // timer running?
        if (--gd->sndtmr != 0) {        // yes, decrement. complete?
            return;                     // no, return
        }
        // timer expired, repeat?
        if (--gd->sndrep != 0) {        // more repeats?
            gd->sndtmr = gd->sndx[1];   // yes, play it
            return sndout(gd->sndx[2]);
        }
        // no more repeats. get next sound
        gd->sndx += 3;                  // next segment
        if (gd->sndx[0] != 0) {         // EOF?
            gd->sndrep = gd->sndx[0];   // no, play it
            gd->sndtmr = gd->sndx[1];
            sndout(gd->sndx[2]);
            return;
        }
        gd->sndpri = 0;                 // nothing playing
    }
    if ((gd->pia21 & 0x2) == 0) {       // thrust off?
        if (gd->thflg) {                // yes, thflg too?
            gd->thflg = 0;              // no, clear it
            sndout(0xf);                // background hum
        }
    } else if (!gd->thflg) {            // still thrusting. thflg set?
        if ((gd->status & 0x98) == 0) { // player alive?
            gd->thflg = 1;              // yes, play thrust
            sndout(0x16);
        }
    }
}

// sound sequence and switch scan
void sndseq(void)
{
    sound_sequencer();
    sscan();
}

void sound_init(void)
{
    const sfx_zheader_t *zh = gd->idata.sfx;
    const sfx_sound_header_t *snd;
    const sfx_header_t *sh;
    static unsigned dlen;
    uint8_t *sfx_data;
    int r, i;

    switch (zh->magic) {
    case SFX_PSG_ZMAGIC:
        snd_drv = &psg_snd_drv;
        gd->issnddma = 0;
        break;
    case SFX_DMA_ZMAGIC:
        if (!(gd->idata.features & IDATA_FEAT_SDMA)) {
            panic(0xadefb954);
        }
        snd_drv = &dma_snd_drv;
        gd->issnddma = 1;
        break;
    default:
        panic(0x22de4351, zh, zh->magic);
    }
    snd_drv->init();

    dlen = zh->uzlen;
    sfx_data = malloc(dlen);
    r = LZG_DecodedSize((uint8_t *)(zh + 1), zh->zlen);
    if (r != dlen) {
        panic(0x431185fb, r, dlen);
    }
    r = LZG_Decode((uint8_t *)(zh + 1), zh->zlen, sfx_data, dlen);
    if (r != dlen) {
        panic(0x4428de99, r, dlen);
    }
    sh = (sfx_header_t *)sfx_data;
    if (sh->magic != SFX_HEADER_MAGIC) {
        panic(0x4fe12210, sh->magic);
    }

    // give the compressed image memory to malloc
    heap_add(gd->idata.sfx, gd->idata.sfxsize);

    nsfx = sh->numsounds;
    sfx = malloc(nsfx * sizeof(*sfx));
    for (i = 0; i < nsfx; i++) {
        snd = (sfx_sound_header_t *)(sfx_data + sh->toc[i].offs);
        if (snd->magic != SFX_MAGIC) {
            panic(0x41bbde12);
        }
        sfx[i].sound = sh->toc[i].sound;
        sfx[i].sptr = (uint8_t *)(snd + 1);
        sfx[i].eptr = sfx[i].sptr + snd->len;
        sfx[i].rptr = NULL;
        sfx[i].max_sptr = sfx[i].sptr;
        snd_drv->sfx_init(&sfx[i], snd->freq, snd->nchans, snd->flags);
    }

    // build the direct index table
    for (i = 0; i < nsfx; i++) {
        if (sfx[i].sound >= SFX_NSOUND_IDS) {
            panic(0x2dace9bb, sfx[i].sound);
        }
        sfxtab[sfx[i].sound] = &sfx[i];
    }
}

void sound_irq_init(void)
{
    snd_drv->irq_init();
}

