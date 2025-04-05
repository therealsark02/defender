/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _MESS_H_
#define _MESS_H_

#include "str_res.h"

extern const uint8_t res_strtab[];

void messf(uint16_t x, uint8_t y, uint16_t strid, ...);

typedef struct {
    const uint8_t *rp;
    uint16_t shiftreg;
    char nbits;
    char prev;
    char backup;
} dec64_t;

static inline void dec64_init(dec64_t *p, uint16_t strid)
{
    p->rp = res_strtab + strid;
    p->backup = 0;
    p->nbits = 0;
}

char dec64_getchar(dec64_t *p);

#endif

