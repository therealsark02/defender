/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SWTCH_H_
#define _SWTCH_H_

#include "keys.h"

extern const char *keynames[KEY__NUM];

typedef struct swdata_s {
    procfn_t paddr;
    uint8_t ptype;
    uint8_t pdata;
} swdata_t;

extern swdata_t swproc[2];

#define KBUFSZ      16              // key buffer size

void sscan(void);
void keyboard_init(void);
void keyboard_os_restore(void);

typedef void (*keyfunc_t)(uint16_t event);
void keyscan(keyfunc_t func);

#define EV_TYPE_SHIFT   9

#define EV_TYPE(e)      ((e) >> EV_TYPE_SHIFT)
enum {
    EV_KEYBOARD,
    EV_JOYSTICK,
    EV_JOYPAD,
};
// keyboard events
#define KEY_EVENT(up, code) \
    ((EV_KEYBOARD << EV_TYPE_SHIFT) | \
    ((up) ? 0x100 : 0) | ((code) & 0xff))

#define EV_KEYUP(e)     (((e) & 0x100) != 0)
#define EV_KEYDN(e)     (((e) & 0x100) == 0)
#define EV_KEY(e)       ((e) & 0xff)

// joystick events
#define JOY_EVENT(j) \
    ((EV_JOYSTICK << EV_TYPE_SHIFT) | ((j) & 0xff))
#define EV_JOY(e)       ((e) & 0xff)

// joypad events
// (left and right d-pad only)
#define JOYPAD_EVENT(j) \
    ((EV_JOYPAD << EV_TYPE_SHIFT) | (((j) >> 5) & 0xf))
#define EV_JPAD(e)          ((e) & 0x0f)
#define EV_JPAD_UP          0x1
#define EV_JPAD_DOWN        0x2
#define EV_JPAD_LEFT        0x4
#define EV_JPAD_RIGHT       0x8

#endif

