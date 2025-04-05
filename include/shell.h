/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SHELLS_H_
#define _SHELLS_H_

void shell(void);
uint8_t bkil(obj_t *obj);
void fbout(obj_t *shl);
obj_t *getshl(obj_t *obj, void (*drawfn)(obj_t *shl),
        const struct sprpat_s *opict, uint8_t (*ocvect)(obj_t *shl));
uint8_t shoot(obj_t *obj);
void shscan(void);

#endif

