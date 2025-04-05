/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SFXDATA_H_
#define _SFXDATA_H_

typedef struct {
    uint16_t magic;
    uint32_t zlen;
    uint32_t uzlen;
} sfx_zheader_t;
#define SFX_PSG_ZMAGIC  0xe330
#define SFX_DMA_ZMAGIC  0xe331

typedef struct {
    uint8_t sound;          // sound id
    uint8_t pad;            // alignment pad
    uint32_t offs;          // offset from start of sfx_header_t
    uint32_t len;           // sound object length
} sfx_toc_entry_t;

typedef struct {
    uint16_t magic;         // magic
    uint32_t totlen;        // total length of header+toc+sound data
    uint8_t numsounds;      // number of sounds
    uint8_t pad;            // alignment pad
    sfx_toc_entry_t toc[0]; // table of contents
} sfx_header_t;
#define SFX_HEADER_MAGIC    0xf5a5

#ifndef M68000
#define SFX_PACKED __attribute__((packed))
#else
#define SFX_PACKED
#endif

typedef struct {
    uint16_t magic;         // magic
    uint16_t freq;          // sample frequency
    uint8_t  flags;         // flags
    uint8_t  nchans;        // nibbles per sample
    uint32_t nsamples;      // number of samples (sound duration)
    uint32_t len;           // data length in bytes
} SFX_PACKED sfx_sound_header_t;
#define SFX_MAGIC   0x5a4f
#define SFXF_PACKED     0x1 // packed 2-nibbles per byte
#define SFXF_REPEAT     0x2 // sample repeats

#endif

