/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _COLLIDE_H_
#define _COLLIDE_H_

struct sprpat_s;

int colide(uint16_t x, uint8_t y, const struct sprpat_s *sprpat);
void colchk(void);

#endif

