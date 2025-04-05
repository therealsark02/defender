/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _OBJECT_H_
#define _OBJECT_H_

struct proc_s;
struct sprpat_s;
struct blip_s;

typedef struct obj_s {
    struct obj_s *olink;
    const struct sprpat_s *opict;
    uint16_t scrx;              // on-screen x (1px prec)
    uint8_t objx;               // on-screen x position (2px prec)
    uint8_t objy;               // on-screen y position (0 == erased)
    void (*erafn)(uint16_t *dst, const uint16_t *src);
    uint16_t *eradst;
    const uint16_t *erasrc;
    struct proc_s *objid;
    uint8_t (*ocvect)(struct obj_s *obj);   // ret 1=hit, 0=ignore
    int16_t ox16;               // world x s10.6 (scrn s10.6 for shell)
    uint16_t oy16;              // y position u8.8
    int16_t oxv;                // s10.6
    int16_t oyv;                // s8.8
    union {
        const struct blip_s *objcol; // object colors in scanner
        void (*drawfn)(struct obj_s *obj);  // draw func for shells
    };
    uint8_t otyp;
    uint8_t odata[2];
} obj_t;

#define NOBJS 95
extern obj_t olist[NOBJS];

obj_t *getob(void);
obj_t *obinit(proc_t *p, const struct sprpat_s *opict,
        uint8_t (*ocvect)(obj_t *obj),
        const struct blip_s *objcol);
uint8_t nokill(obj_t *obj);
void killob(obj_t *obj);
void kilshl(obj_t *obj);
void kiloff(obj_t *obj);
void killop(obj_t *obj);
void kilpos(obj_t *obj, uint16_t pts, const uint8_t *snd);
void kilos(obj_t *obj, uint16_t scr, const uint8_t *snd);

void oinit(void);
void oproc(void);
void velo(void);
void oscan(void);
void iscan(void);
void ofshit(obj_t *obj);

#endif

