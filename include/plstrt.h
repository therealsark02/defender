/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _PLSTRT_H_
#define _PLSTRT_H_

typedef struct pldata_s {
    uint32_t pscor;         // score (only [23:0] used)
    uint32_t prpla;         // next replay level (only [23:0])
    uint8_t plas;           // #of lasers
    uint8_t pwav;           // wave # in hex
    uint8_t psbc;           // smart bombs
    uint8_t ptarg;          // astronauts
    uint8_t penemy[50];     // enemy state
} pldata_t;
extern pldata_t pldata[];

static inline pldata_t *plindx(void)
{
    return &pldata[gd->curplr - 1];
}

static inline pldata_t *pldx(uint8_t i)
{
    return &pldata[i - 1];
}

static inline void stchka(uint8_t status, uint8_t astcnt)
{
    if (astcnt == 0) {
        status |= ST_NOTERRAIN;
    }
    gd->status = status;
}

static inline void stchk0(uint8_t status)
{
    stchka(status, gd->astcnt);
}

static inline void stchk(void)
{
    stchk0(0);
}

void st1(void);
void st2(void);
void init20(void);
void wdelt(uint8_t *dst, uint8_t col);
void getwv(pldata_t *pyr);
void plsav(void);
void plstr0(void) __dead;
void plstrt(void) __dead;

#endif

