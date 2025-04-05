/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _BLIPS_H_
#define _BLIPS_H_

// NOTE: B_8888 is now B_7777
#define BLIP_GENERATOR(mac) \
    mac(00,00) \
    mac(24,24) \
    mac(33,33) \
    mac(44,33) \
    mac(66,66) \
    mac(77,77) \
    mac(CC,33) \
    mac(CC,CC)

#define BLIP_ENUM_GEN(a, b) B_##a##b,
enum {
    BLIP_GENERATOR(BLIP_ENUM_GEN)
    B__NUM
};

typedef struct blip_s {
    uint32_t pat[8][4];         // 8 shifted patterns
} blip_t;

extern blip_t bliptab[B__NUM];

void blips_init(void);
void blips_erase(void);
void blips_plot(uint16_t offsx);

extern const uint32_t blipmask[8];

#endif

