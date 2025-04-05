/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _YM2149_H_
#define _YM2149_H_

#define YM_SELECT   0xffff8800
#define YM_DATA     0xffff8802

#define YM_AFINE    0x00
#define YM_ACOARSE  0x01
#define YM_BFINE    0x02
#define YM_BCOARSE  0x03
#define YM_CFINE    0x04
#define YM_CCOARSE  0x05
#define YM_NOISEPER 0x06
#define YM_ENABLE   0x07
#define YM_AVOL     0x08
#define YM_BVOL     0x09
#define YM_CVOL     0x0a
#define YM_EFINE    0x0b
#define YM_ECOARSE  0x0c
#define YM_ESHAPE   0x0d
#define YM_PORTA    0x0e
#define YM_PORTB    0x0f

#define YM_PORTA_NDRIVE1 0x04
#define YM_PORTA_NDRIVE0 0x02
#define YM_PORTA_SIDE0   0x01
#define YM_PORTA_SIDE1   0x00

#define YM_PORTA_FDC(m, s) \
    ((((m) == 0x1) ? YM_PORTA_NDRIVE1 : \
    ((m) == 0x2) ? YM_PORTA_NDRIVE0 : \
    (YM_PORTA_NDRIVE0 | YM_PORTA_NDRIVE1)) | \
    ((s) ? YM_PORTA_SIDE1 : YM_PORTA_SIDE0))

#ifndef __ASSEMBLER__
static inline void ym_write(int addr, uint8_t val)
{
    asm volatile("movep.w %0,0(%1)" ::
            "d"((addr << 8) | val), "a"(YM_SELECT));
}
#endif

#endif

