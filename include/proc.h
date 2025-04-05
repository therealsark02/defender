/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _PROC_H_
#define _PROC_H_

// proc.c
typedef void (*procfn_t)(void);

struct sprpat_s;

typedef struct proc_s {
    struct proc_s *plink;           // +00
    procfn_t paddr;                 // +04
    uint8_t ptime;                  // +08
    uint8_t ptype;                  // +09
    uint8_t pcod;
    union {
        uint8_t pd[8];
        struct {
            uint8_t a;
            uint8_t b;
            uint16_t x;
            uint8_t shift;
            uint8_t cnt;
            uint32_t *ptr;
            const uint32_t (*colptr0)[2];
            const uint32_t (*colptr1)[2];
            uint32_t mask;
        } ramtest;
        struct {
            union {
                struct obj_s **tgt; // pd
                struct obj_s *obj; // pd
            };
        } astro;
        struct {
            struct obj_s *obj;      // pd
            struct obj_s *tobj;     // pd2
            struct obj_s **tptr;    // pd4
            uint8_t stim;           // pd6
        } lander;
        struct {
            struct obj_s *obj;      // pd
            uint8_t stim;           // pd2
        } schitzo;
        struct {
            struct obj_s *obj;      // pd
            uint8_t stim;           // pd2
        } ufo;
        struct {
            struct obj_s *obj;      // pd
            uint8_t accel;          // pd2
            uint8_t stim;           // pd4
        } swarmer;
        struct {
            struct obj_s *obj;      // pd
        } p500;
        struct {
            laser_t pd[3];
            uint16_t *eol;
            uint8_t x;
            uint8_t y;
            uint8_t tailcnt;
        } laser;
        struct {
            uint8_t cnt;    // pd
        } terblo;
        struct {
            uint16_t x;         // pd
            uint8_t astcnt;     // pd2
            void (*func)(void); // pd+6
        } bonus;
        struct {
            uint8_t cnt;        // pd
        } sbomb;
        struct {
            struct obj_s *obj;  // pd
        } hyper;
        struct {
            const uint8_t *col;         // pd
            const struct sprpat_s *spr; // pd+4
        } plend;
        struct {
            void (*func)(void);         // pd2
        } plex;
    };
} proc_t;
#define STYPE   0x0
#define AMTYPE  0x0
#define ATYPE   0x1
#define CTYPE   0x2

#define NPROCS      75
extern proc_t ptab[NPROCS];

typedef struct sproc_s {
    proc_t *plink;
    void (*paddr)(void);
    uint8_t ptime;
    uint8_t ptype;
    uint8_t pcod;
    union {
        uint8_t pd[16];
        struct {
            struct obj_s *tie[4];
            uint8_t nsquad;
            uint8_t cralt;
        } tie;
    };
} sproc_t;

#define NSPROCS     5
extern sproc_t sptab[NSPROCS];

void pinit(void);
proc_t *mkproc(procfn_t fn, uint8_t ptype);
sproc_t *msproc(procfn_t fn, uint8_t ptype);
void gncide(void);
proc_t *kill(proc_t *p);
void sucide(void) __dead;
void sleep(procfn_t fn, uint8_t ptime) __dead;
void disp(void) __dead;

#endif

