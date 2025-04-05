/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#define EJP_FIRE    0xffff9200
#define EJP_STICK   0xffff9202
#define EJP_PAD0_X  0xffff9210
#define EJP_PAD0_Y  0xffff9212
#define EJP_PAD1_X  0xffff9214
#define EJP_PAD1_Y  0xffff9216
#define EJP_GUN_X   0xffff9222
#define EJP_GUN_Y   0xffff9222

#define EJP_READGROUP(enable, firemask, fireshift, stickshift) \
    do { \
        writew(EJP_STICK, (enable)); \
        res |= (~readw(EJP_FIRE) & (firemask)) << (fireshift); \
        res |= ((~readw(EJP_STICK) & 0xf00) >> 8) << (stickshift); \
    } while (0)

#define JOYPAD_GENERATOR(mac) \
    mac(PAUSE) \
    mac(A) \
    mac(B) \
    mac(C) \
    mac(OPTION) \
    mac(UP) \
    mac(DOWN) \
    mac(LEFT) \
    mac(RIGHT) \
    mac(KPAST) \
    mac(KP7) \
    mac(KP4) \
    mac(KP1) \
    mac(KP0) \
    mac(KP8) \
    mac(KP5) \
    mac(KP2) \
    mac(KPHASH) \
    mac(KP9) \
    mac(KP6) \
    mac(KP3)

#define JOYPAD_ENUM_GEN(a) JOYPAD_##a,

enum {
    JOYPAD_GENERATOR(JOYPAD_ENUM_GEN)
    JOYPAD__NUM
};

#define JOYPAD_BIT_GEN(a) JOYPAD_##a##_BIT = (1 << JOYPAD_##a),
enum {
    JOYPAD_GENERATOR(JOYPAD_BIT_GEN)
};

static inline uint32_t joypad_read(void)
{
    uint32_t res = 0;

    EJP_READGROUP(0xfffe, 0x3, 0,  5);
    EJP_READGROUP(0xfffd, 0x2, 1,  9);
    EJP_READGROUP(0xfffb, 0x2, 2, 13);
    EJP_READGROUP(0xfff7, 0x2, 3, 17);

    return res;
}

#endif

