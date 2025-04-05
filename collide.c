/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Object-sprite Collision functions.
 */

#include "gd.h"
#include "sprites.h"
#include "plot.h"
#include "obj.h"
#include "plend.h"
#include "collide.h"

static int col0(obj_t *obj, uint16_t this_x, uint8_t this_y, const sprpat_t *this_sprpat)
{
    uint8_t obj_y, obj_w, obj_h, this_w, this_h, uly, lry;
    uint16_t this_align_x, obj_align_x;
    const uint16_t *this_src;
    uint16_t obj_x, ulx, lrx;
    uint8_t this_row, obj_row;
    int8_t nrows, dtop, dbot;
    const uint16_t *obj_src;

    this_w = this_sprpat->w;
    this_h = this_sprpat->h;

    ulx = this_x;
    uly = this_y;
    lrx = this_x + this_w;
    lry = this_y + this_h;

    for (; obj != NULL; obj = obj->olink) {
        // simple rectangle intersection
        obj_x = obj->scrx;
        obj_y = obj->objy;
        obj_w = obj->opict->w;
        obj_h = obj->opict->h;
        if (obj_y == 0 ||               // off screen
            obj_x >= lrx ||             // off right
            obj_y >= lry) {             // below
            continue;
        }
        if (obj_x + obj_w <= ulx ||     // off left
            obj_y + obj_h <= uly) {     // above
            continue;
        }

        // check picture intersect
        dtop = this_y - obj_y;
        dbot = (this_y + this_h) - (obj_y + obj_h);
        if (dtop <= 0) {
            this_row = -dtop;
            obj_row = 0;
            // this: |------
            // obj:      |--
            if (dbot <= 0) {
                // this: |------|
                // obj:      |------|
                nrows = this_y + this_h - obj_y;
            } else {
                // this: |----------|
                // obj:      |---|
                nrows = obj_h;
            }
        } else {
            this_row = 0;
            obj_row = dtop;
            // this:     |--
            // obj:  |------
            if (dbot <= 0) {
                // this:     |----|
                // obj:  |-----------|
                nrows = this_h;
            } else {
                // this:     |-------|
                // obj:  |--------|
                nrows = obj_y + obj_h - this_y;
            }
        }
        if (nrows <= 0) {
            panic(0x3ef45233, nrows);
        }
        if (this_row >= this_h || this_row + nrows > this_h) {
            panic(0xaec3e544, this_row, this_h, nrows);
        }
        if (obj_row >= obj_h || obj_row + nrows > obj_h) {
            panic(0xaec3e5cc, obj_row, obj_h, nrows);
        }

        this_src = this_sprpat->pat[this_x & 0xf];
        this_src += 12 * this_row;
        obj_src = obj->opict->pat[obj_x & 0xf];
        obj_src += 12 * obj_row;

        this_align_x = this_x & -16;
        obj_align_x = obj_x & -16;
        if (this_align_x < obj_align_x) {
            // this:  |------|------|
            // obj:          |------|------|
            this_src += 6;
            if (!spr_collide_half(this_src, obj_src, nrows)) {
                continue;
            }
        } else if (this_align_x > obj_align_x) {
            // this:        |------|------|
            // obj:  |------|------|
            obj_src += 6;
            if (!spr_collide_half(this_src, obj_src, nrows)) {
                continue;
            }
        } else {
            // this:  |------|------|
            // obj:   |------|------|
            if (!spr_collide_full(this_src, obj_src, nrows)) {
                continue;
            }
        }
        return obj->ocvect(obj);
    }
    return 0;
}

int colide(uint16_t x, uint8_t y, const sprpat_t *sprpat)
{
    return col0(gd->optr, x, y, sprpat);
}

void colchk(void)
{
    const sprpat_t *sprpat;

    if (!(gd->status & 0x10)) {
        if (gd->pladir < 0) {
            sprpat = &sprtab[P_PLBPIC];
        } else {
            sprpat = &sprtab[P_PLAPIC];
        }
        ++gd->pcflg;
        if ((colide(gd->plascrx, gd->playc, sprpat) ||
             col0(gd->sptr, gd->plascrx, gd->playc, sprpat)) &&
            !(gd->cheats & CHEAT_INVINCIBLE)) {
            mkproc(plend, STYPE);
            gd->status |= ST_PLDEAD;
        }
    }
    gd->pcflg = 0;
}

