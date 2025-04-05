/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _MENU_H_
#define _MENU_H_

struct menu_s;

typedef struct {
    int name;
    union {
        void (*func)(uint8_t id);
        const struct menu_s *menu;
    };
    uint16_t attrs;
} mitem_t;
#define M_ATTR_MENU     0x1
#define M_ATTR_APPONLY  0x2
#define M_ATTR_DEVONLY  0x4

typedef struct menu_s {
    const mitem_t *items;
    uint16_t nitems;
    void (*func)(void);
    uint8_t yoffs;
} menu_t;

#define _ITEM_FUNC(nm, fn, at)   { (nm), .func = (fn), (at) }
#define ITEM_FUNC(nm, fn)        _ITEM_FUNC(nm, fn, 0)
#define ITEM_FUNC_APPONLY(nm, fn) _ITEM_FUNC(nm, fn, M_ATTR_APPONLY)

#define _ITEM_MENU(nm, mn, at)   { (nm), .menu = (mn), (at) | M_ATTR_MENU }
#define ITEM_MENU(nm, mn)        _ITEM_MENU(nm, mn, 0)
#define ITEM_MENU_DEVONLY(nm, mn) _ITEM_MENU(nm, mn, M_ATTR_DEVONLY)

#define MENU_SPEC(nm, it, fn, yp) \
static const menu_t nm = { \
    (it), sizeof(it) / sizeof((it)[0]), \
    (fn), (yp) \
}
    
#define MENU(nm, it)             MENU_SPEC(nm, it, 0, 0)

#endif

