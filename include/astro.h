/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _ASTRO_H_
#define _ASTRO_H_

#define NTLIST 20
extern obj_t *tlist[NTLIST];

void astro(void);
uint8_t astkil(obj_t *obj);
uint8_t astk1(obj_t *obj);
uint8_t akil1(obj_t *obj);
void astst(proc_t *p, uint8_t cnt, uint8_t hi_x, obj_t **tgt);
void afall(void);

#endif

