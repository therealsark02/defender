/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _MKSPRITES_H_
#define _MKSPRITES_H_

extern const pict_t **pictab;
extern sprpat_t *sprtab;
extern int npictab;

#define N_PTHRUST       12
#define N_PFIREBALL     12

#define P_PLYRR         NALLPICTS
#define P_PLYRL         (P_PLYRR + N_PTHRUST * 2)
#define P_FIREBALL      (P_PLYRL + N_PTHRUST * 2)
#define P_PLAYERBLIP    (P_FIREBALL + N_PFIREBALL)
#define P__NUM          (P_PLAYERBLIP + 1)

void mksprites(void);
void mkchars(void);

// characters
extern sprpat_t *sprchrtab;

#endif

