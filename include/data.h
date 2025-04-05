/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _DATA_H_
#define _DATA_H_

typedef struct pict_s {
    uint8_t w;                  // width in units of 2 pixels
    uint8_t h;                  // height
    const uint8_t *pat;         // even xpos pattern
} pict_t;

enum {
    C_NUMBR0,
    C_NUMBR1,
    C_NUMBR2,
    C_NUMBR3,
    C_NUMBR4,
    C_NUMBR5,
    C_NUMBR6,
    C_NUMBR7,
    C_NUMBR8,
    C_NUMBR9,
    C_BLANK3X8,
    C__NUM,
};
#define NCHRTBL     C__NUM
extern const pict_t chrtbl[NCHRTBL];

enum {
    P_SCZP1,
    P_ASXP1,
    P_SWXP1,
    P_NULOB,
    P_PRBP1,
    P_ASTP1,
    P_ASTP2,
    P_ASTP3,
    P_ASTP4,
    P_TIEP1,
    P_TIEP2,
    P_TIEP3,
    P_TIEP4,
    P_BXPIC,
    P_BMBP1,
    P_BMBP2,
    P_LASP1,
    P_SWPIC1,
    P_LNDP1,
    P_LNDP2,
    P_LNDP3,
    P_UFOP1,
    P_UFOP2,
    P_UFOP3,
    P_PLAMIN,
    P_SBPIC,
    P_C25P1,
    P_C5P1,
    P_TEREX,
    P_PLAPIC,
    P_PLBPIC,
    P_PLAPICM,
    P_PLBPICM,
    NALLPICTS,
};
extern const pict_t * const allpicts[NALLPICTS];

extern const pict_t plapic;
extern const pict_t plbpic;

extern const uint8_t lgotab[];
extern const uint8_t defdat[];
extern const uint32_t defdatlen;
extern const uint8_t dcolrs[];

extern const uint16_t waitimg[];
extern const uint16_t waitimg_nw;
extern const uint16_t waitimg_x;
extern const uint16_t waitimg_h;

extern const uint16_t helpimg[];
extern const uint16_t helpimg_nw;
extern const uint16_t helpimg_x;
extern const uint16_t helpimg_h;

extern const uint16_t blurbimg[];
extern const uint16_t blurbimg_nw;
extern const uint16_t blurbimg_x;
extern const uint16_t blurbimg_h;

#endif

