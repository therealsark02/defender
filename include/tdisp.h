/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _TDISP_H_
#define _TDISP_H_

void flpup(void);
void tdisp(void);
void score(uint16_t pts);
void scrtrn(void);
void scrtr0(uint8_t pid);
void sbdisp(void);
void border(void);

void cwrit(uint16_t x, uint8_t y, const sprpat_t *spr);
void coff(uint16_t x, uint8_t y, const sprpat_t *spr);
void blkclr(uint16_t x, uint8_t y, uint16_t w, uint8_t h);

#endif

