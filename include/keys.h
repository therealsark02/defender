/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _KEYS_H_
#define _KEYS_H_

#define KEY_GENERATOR(mac) \
    /*  only keys valid for control */ \
    /*  binding have a name         */ \
    /*  key         code  name      */ \
    mac(ESC,           1, "ESC") \
    mac(1,             2, "1") \
    mac(2,             3, "2") \
    mac(3,             4, "3") \
    mac(4,             5, "4") \
    mac(5,             6, "5") \
    mac(6,             7, "6") \
    mac(7,             8, "7") \
    mac(8,             9, "8") \
    mac(9,            10, "9") \
    mac(0,            11, "0") \
    mac(MINUS,        12, "-") \
    mac(EQUAL,        13, "=") \
    mac(BACKSPACE,    14, "BKSPC") \
    mac(TAB,          15, "TAB") \
    mac(Q,            16, "Q") \
    mac(W,            17, "W") \
    mac(E,            18, "E") \
    mac(R,            19, "R") \
    mac(T,            20, "T") \
    mac(Y,            21, "Y") \
    mac(U,            22, "U") \
    mac(I,            23, "I") \
    mac(O,            24, "O") \
    mac(P,            25, "P") \
    mac(LBRACE,       26, "{") \
    mac(RBRACE,       27, "}") \
    mac(ENTER,        28, "ENTER") \
    mac(LCTRL,        29, "LCTRL") \
    mac(A,            30, "A") \
    mac(S,            31, "S") \
    mac(D,            32, "D") \
    mac(F,            33, "F") \
    mac(G,            34, "G") \
    mac(H,            35, "H") \
    mac(J,            36, "J") \
    mac(K,            37, "K") \
    mac(L,            38, "L") \
    mac(SEMICOLON,    39, ";") \
    mac(APOSTROPHE,   40, "'") \
    mac(GRAVE,        41, "`") \
    mac(LSHIFT,       42, "LSHIFT") \
    mac(BACKSLASH,    43, "\\") \
    mac(Z,            44, "Z") \
    mac(X,            45, "X") \
    mac(C,            46, "C") \
    mac(V,            47, "V") \
    mac(B,            48, "B") \
    mac(N,            49, "N") \
    mac(M,            50, "M") \
    mac(COMMA,        51, ",") \
    mac(DOT,          52, ".") \
    mac(SLASH,        53, "/") \
    mac(RSHIFT,       54, "RSHIFT") \
    mac(KPASTERISK,   55, "KP*") \
    mac(LALT,         56, "LALT") \
    mac(SPACE,        57, "SPACE") \
    mac(CAPSLOCK,     58, "CAPS") \
    mac(F1,           59, NULL) \
    mac(F2,           60, NULL) \
    mac(F3,           61, NULL) \
    mac(F4,           62, NULL) \
    mac(F5,           63, NULL) \
    mac(F6,           64, NULL) \
    mac(F7,           65, NULL) \
    mac(F8,           66, NULL) \
    mac(F9,           67, NULL) \
    mac(F10,          68, NULL) \
    mac(HOME,         71, "HOME") \
    mac(UP,           72, "UP") \
    mac(KPMINUS,      74, "KP-") \
    mac(LEFT,         75, "LEFT") \
    mac(RIGHT,        77, "RIGHT") \
    mac(KPPLUS,       78, "KP+") \
    mac(DOWN,         80, "DOWN") \
    mac(INSERT,       82, "INS") \
    mac(DELETE,       83, "DEL") \
    mac(102ND,        96, NULL) \
    mac(UNDO,         97, "UNDO") \
    mac(HELP,         98, "HELP") \
    mac(KPLPAREN,     99, "KP(") \
    mac(KPRPAREN,    100, "KP)") \
    mac(KPSLASH,     101, "KP/") \
    mac(KPASTERISK2, 102, "KP*") \
    mac(KP7,         103, "KP7") \
    mac(KP8,         104, "KP8") \
    mac(KP9,         105, "KP9") \
    mac(KP4,         106, "KP4") \
    mac(KP5,         107, "KP5") \
    mac(KP6,         108, "KP6") \
    mac(KP1,         109, "KP1") \
    mac(KP2,         110, "KP2") \
    mac(KP3,         111, "KP3") \
    mac(KP0,         112, "KP0") \
    mac(KPDOT,       113, "KP.") \
    mac(KPENTER,     114, "KPENT") \
    mac(JPAD_PAUSE,  128, "JPADP") \
    mac(JPAD_A,      129, "JPADA") \
    mac(JPAD_B,      130, "JPADB") \
    mac(JPAD_C,      131, "JPADC") \
    mac(JPAD_OPTION, 132, "JPADOP") \
    mac(JPAD_UP,     133, "JPADUP") \
    mac(JPAD_DOWN,   134, "JPADDN") \
    mac(JPAD_LEFT,   135, "JPADLT") \
    mac(JPAD_RIGHT,  136, "JPADRT") \
    mac(JPAD_KPAST,  137, "JPAD*") \
    mac(JPAD_KP7,    138, "JPAD7") \
    mac(JPAD_KP4,    139, "JPAD4") \
    mac(JPAD_KP1,    140, "JPAD1") \
    mac(JPAD_KP0,    141, "JPAD0") \
    mac(JPAD_KP8,    142, "JPAD8") \
    mac(JPAD_KP5,    143, "JPAD5") \
    mac(JPAD_KP2,    144, "JPAD2") \
    mac(JPAD_KPHASH, 145, "JPAD#") \
    mac(JPAD_KP9,    146, "JPAD9") \
    mac(JPAD_KP6,    147, "JPAD6") \
    mac(JPAD_KP3,    148, "JPAD3")

#define KEY_ENUM_GEN(a, b, c) KEY_##a = b,
enum {
    KEY_GENERATOR(KEY_ENUM_GEN)
    KEY__NUM
};

#define K_DN(n)     (KEY_##n)
#define K_UP(n)     (0x100 | KEY_##n)

#define JOY_FIRE    0x80
#define JOY_RIGHT   0x08
#define JOY_LEFT    0x04
#define JOY_DOWN    0x02
#define JOY_UP      0x01

#endif

