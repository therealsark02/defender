/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _MFP_H_
#define _MFP_H_

// MFP Registers
#define _MFP_GPIP 0x01
#define _MFP_AER  0x03
#define _MFP_DDR  0x05
#define _MFP_IERA 0x07
#define _MFP_IERB 0x09
#define _MFP_IPRA 0x0b
#define _MFP_IPRB 0x0d
#define _MFP_IISA 0x0f
#define _MFP_IISB 0x11
#define _MFP_IMRA 0x13
#define _MFP_IMRB 0x15
#define _MFP_VR   0x17
#define _MFP_TCRA 0x19
#define _MFP_TCRB 0x1b
#define _MFP_TCCD 0x1d
#define _MFP_TDRA 0x1f
#define _MFP_TDRB 0x21
#define _MFP_TDRC 0x23
#define _MFP_TDRD 0x25
#define _MFP_SCR  0x27
#define _MFP_UCR  0x29
#define _MFP_RSR  0x2b
#define _MFP_TSR  0x2d
#define _MFP_UDR  0x2f

#define MFP_BASE 0xfffffa00
#define MFP_GPIP (MFP_BASE + _MFP_GPIP)
#define MFP_AER  (MFP_BASE + _MFP_AER)
#define MFP_DDR  (MFP_BASE + _MFP_DDR)
#define MFP_IERA (MFP_BASE + _MFP_IERA)
#define MFP_IERB (MFP_BASE + _MFP_IERB)
#define MFP_IPRA (MFP_BASE + _MFP_IPRA)
#define MFP_IPRB (MFP_BASE + _MFP_IPRB)
#define MFP_IISA (MFP_BASE + _MFP_IISA)
#define MFP_IISB (MFP_BASE + _MFP_IISB)
#define MFP_IMRA (MFP_BASE + _MFP_IMRA)
#define MFP_IMRB (MFP_BASE + _MFP_IMRB)
#define MFP_VR   (MFP_BASE + _MFP_VR)
#define MFP_TCRA (MFP_BASE + _MFP_TCRA)
#define MFP_TCRB (MFP_BASE + _MFP_TCRB)
#define MFP_TCCD (MFP_BASE + _MFP_TCCD)
#define MFP_TDRA (MFP_BASE + _MFP_TDRA)
#define MFP_TDRB (MFP_BASE + _MFP_TDRB)
#define MFP_TDRC (MFP_BASE + _MFP_TDRC)
#define MFP_TDRD (MFP_BASE + _MFP_TDRD)
#define MFP_SCR  (MFP_BASE + _MFP_SCR)
#define MFP_UCR  (MFP_BASE + _MFP_UCR)
#define MFP_RSR  (MFP_BASE + _MFP_RSR)
#define MFP_TSR  (MFP_BASE + _MFP_TSR)
#define MFP_UDR  (MFP_BASE + _MFP_UDR)

#define MFP_TMR_DIV_4       0x1
#define MFP_TMR_DIV_10      0x2
#define MFP_TMR_DIV_16      0x3
#define MFP_TMR_DIV_50      0x4
#define MFP_TMR_DIV_64      0x5
#define MFP_TMR_DIV_100     0x6
#define MFP_TMR_DIV_200     0x7

#ifndef __ASSEMBLER__

#define DECL_MFP_SETCLR(n, r) \
    static inline void mfp_##n##_set(uint8_t mask) \
    { \
        writeb(r, readb(r) | (mask)); \
    } \
    static inline void mfp_##n##_clr(uint8_t mask) \
    { \
        writeb(r, readb(r) & ~(mask)); \
    }
DECL_MFP_SETCLR(iera, MFP_IERA)
DECL_MFP_SETCLR(imra, MFP_IMRA)
DECL_MFP_SETCLR(iisa, MFP_IISA)
DECL_MFP_SETCLR(ierb, MFP_IERB)
DECL_MFP_SETCLR(imrb, MFP_IMRB)
DECL_MFP_SETCLR(iisb, MFP_IISB)
#endif

#endif

