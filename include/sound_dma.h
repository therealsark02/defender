/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SOUND_DMA_H_
#define _SOUND_DMA_H_

// Sound DMA Registers
#define SDMA_CTRL       0xffff8901      // byte
#define SDMA_STARTH     0xffff8903      // byte
#define SDMA_STARTM     0xffff8905      // byte
#define SDMA_STARTL     0xffff8907      // byte
#define SDMA_COUNTERH   0xffff8909      // byte
#define SDMA_COUNTERM   0xffff890b      // byte
#define SDMA_COUNTERL   0xffff890d      // byte
#define SDMA_ENDH       0xffff890f      // byte
#define SDMA_ENDM       0xffff8911      // byte
#define SDMA_ENDL       0xffff8913      // byte
#define SDMA_MODE       0xffff8920      // word
#define SDMA_UWIREDATA  0xffff8922      // word
#define SDMA_UWIREMASK  0xffff8924      // word

#define SDMA_CTRL_DISABLE   0x00
#define SDMA_CTRL_ENABLE    0x01
#define SDMA_CTRL_REPEAT    0x02

#define SDMA_MODE_MONO      0x0080
#define SDMA_MODE_STEREO    0x0000
#define SDMA_MODE_50066HZ   0x0003
#define SDMA_MODE_25033HZ   0x0002
#define SDMA_MODE_12517HZ   0x0001
#define SDMA_MODE_6258HZ    0x0000

static inline uint32_t sdma_counter_get(void)
{
    uint8_t *p = (uint8_t *)0xffff8906; // STARTL
    uint32_t val;

    asm volatile("movep.l 0(%1),%0" : "=r"(val) : "a"(p) : "cc");
    return val & 0xffffff;
}

static inline void sdma_start_set(uint32_t addr)
{
    uint8_t *p = (uint8_t *)SDMA_STARTH;

    asm volatile(
        "move.b %1,(%0)\n"
        "movep.w %2,2(%0)"
        :: "a"(p), "r"(addr >> 16), "r"(addr) : "cc");
}

static inline void sdma_end_set(uint32_t addr)
{
    uint8_t *p = (uint8_t *)SDMA_ENDH;

    asm volatile(
        "move.b %1,(%0)\n"
        "movep.w %2,2(%0)"
        :: "a"(p), "r"(addr >> 16), "r"(addr) : "cc");
}

#endif

