/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include "gd.h"
#include "font.h"

#define _ 0
#define O 1
#define BIN(a, b, c, d, e, f, g, h) \
    (((a) << 7) | ((b) << 6) | ((c) << 5) | \
     ((d) << 4) | ((e) << 3) | ((f) << 2) | \
     ((g) << 1) | ((h) << 0))

static const uint8_t atari_glyphs[] = {
    BIN(  O,O,O,O,O,O,O,O ),    // 0x20
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  O,O,O,O,O,O,O,O ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x21
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x22
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x23
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,O,O,_,_,_ ),    // 0x24
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x25
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,O,O,O,_,_,_ ),    // 0x26
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,O,O,O,_,_,_,_ ),
    BIN(  O,O,_,O,O,O,O,O ),
    BIN(  O,O,_,_,O,O,_,_ ),
    BIN(  _,O,O,O,_,O,O,O ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x27
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x28
    BIN(  _,_,_,_,O,O,O,_ ),
    BIN(  _,_,_,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,O,_,_ ),
    BIN(  _,_,_,_,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x29
    BIN(  _,O,O,O,_,_,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,O,O,O,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2a
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  O,O,O,O,O,O,O,O ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2b
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2c
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2d
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2e
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x2f
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x30
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,O,_ ),
    BIN(  _,O,O,O,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x31
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x32
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x33
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x34
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,O,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x35
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x36
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x37
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x38
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x39
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x3a
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x3b
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),

    BIN(  _,_,_,_,_,O,O,_ ),    // 0x3c
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x3d
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,O,O,_,_,_,_,_ ),    // 0x3e
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x3f
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x40
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,O,_ ),
    BIN(  _,O,O,_,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x41
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x42
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x43
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x44
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x45
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x46
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x47
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x48
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x49
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4a
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4b
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4c
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4d
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,O,O,O,_,O,O,O ),
    BIN(  _,O,O,O,O,O,O,O ),
    BIN(  _,O,O,_,O,_,O,O ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4e
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x4f
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x50
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x51
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,_,O,O,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x52
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x53
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x54
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x55
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x56
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x57
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,O,O,_,O,_,O,O ),
    BIN(  _,O,O,O,O,O,O,O ),
    BIN(  _,O,O,O,_,O,O,O ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x58
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x59
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5a
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5b
    BIN(  _,_,_,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5c
    BIN(  _,O,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5d
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5e
    BIN(  _,_,_,_,O,_,_,_ ),
    BIN(  _,_,_,O,O,O,_,_ ),
    BIN(  _,_,O,O,_,O,O,_ ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x5f
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x60
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x61
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x62
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x63
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x64
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x65
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x66
    BIN(  _,_,_,_,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x67
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x68
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x69
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6a
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6b
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),
    BIN(  _,O,O,_,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6c
    BIN(  _,_,O,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6d
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,O,O ),
    BIN(  _,O,O,O,O,O,O,O ),
    BIN(  _,O,O,_,O,_,O,O ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6e
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x6f
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x70
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x71
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x72
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x73
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,O,O,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x74
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x75
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x76
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x77
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,O,O ),
    BIN(  _,O,O,_,O,_,O,O ),
    BIN(  _,O,O,O,O,O,O,O ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,O,O,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x78
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,O,O,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x79
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,O,O,_,_,O,O,_ ),
    BIN(  _,_,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,O,O,O,O,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x7a
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,O,O,_ ),    // 0x7b
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,O,O,_,_ ),
    BIN(  _,_,_,_,_,O,O,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x7c
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,O,O,_,_,_,_,_ ),    // 0x7d
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,_,O,O,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,_,O,O,_,_,_,_ ),
    BIN(  _,O,O,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),

    BIN(  _,_,_,_,_,_,_,_ ),    // 0x7e
    BIN(  _,_,O,O,_,_,O,O ),
    BIN(  _,O,O,O,O,O,O,_ ),
    BIN(  O,O,_,O,O,O,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
    BIN(  _,_,_,_,_,_,_,_ ),
};
#undef _
#undef O

const font_t atari_font = {
    .glyphs = atari_glyphs,
    .fixed_width = 8,
    .charsp = 0
};

