/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _EXPL_H_
#define _EXPL_H_

typedef struct {
    uint32_t pat[8][4];
} fragpat_t;

typedef struct {
    uint8_t w;
    uint8_t h;
    fragpat_t *fragpat[0];  // [col][row]
} fragdesc_t;

enum {
    EX_NONE,
    EX_SCZP1,   // 5x4 = 20 frags
    EX_ASXP1,   // 4x4 = 16 frags
    EX_SWXP1,   // 4x4 = 16 frags
    EX_PRBP1,   // 4x4 = 16 frags
    EX_ASTP2,   // 2x4 =  8 frags
    EX_ASTP4,   // 2x4 =  8 frags
    EX_TIEP1,   // 4x4 = 16 frags
    EX_BXPIC,   // 4x4 = 16 frags
    EX_SWPIC1,  // 3x2 =  6 frags
    EX_LNDP1,   // 5x4 = 20 frags
    EX_UFOP1,   // 6x2 = 12 frags
    EX_TEREX,   // 8x3 = 24 frags
    EX_PLAPIC,  // 8x3 = 24 frags
    EX_PLBPIC,  // 8x3 = 24 frags
    EX__NUM
};

// max number of frags to erase
#define EX_MAXFRAGS 24

struct sprpat_s;
struct obj_s;

typedef struct {
    int16_t rsize;
    const struct sprpat_s *obdesc;
    uint32_t **erases;
    uint8_t toplft_x;       // 2px
    uint8_t toplft_y;
    uint8_t center_x;       // 2px
    uint8_t center_y;
    struct obj_s *objptr;
    const fragdesc_t *fragdesc;
    uint32_t *erasetab[EX_MAXFRAGS];
} expl_t;

// explosion ram allocation
#define NRAMALS 16

void expl_init(void);
void mkfrag(fragpat_t *fp, uint8_t a, uint8_t b);

void apst(struct obj_s *obj);      // appear start
void exst(struct obj_s *obj);      // explosion start
void expu(void);            // explosion update

#endif

