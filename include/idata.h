/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _IDATA_H_
#define _IDATA_H_

#define IDATA_MACH_GENERATOR(mac) \
    mac(UNKNOWN) \
    mac(ST) \
    mac(STE) \
    mac(MEGA_STE) \
    mac(FALCON)

#define IDATA_MACH_ENUM_GEN(a) IDATA_MACH_##a,
enum {
    IDATA_MACH_GENERATOR(IDATA_MACH_ENUM_GEN)
    IDATA_MACH__NUM
};

#define IDATA_CPU_GENERATOR(mac) \
    mac(UNKNOWN) \
    mac(68000) \
    mac(68030)

#define IDATA_CPU_ENUM_GEN(a) IDATA_CPU_##a,
enum {
    IDATA_CPU_GENERATOR(IDATA_CPU_ENUM_GEN)
    IDATA_CPU__NUM
};

#define IDATA_VID_GENERATOR(mac) \
    mac(UNKNOWN) \
    mac(ST) \
    mac(STE) \
    mac(FALCON)

#define IDATA_VID_ENUM_GEN(a) IDATA_VID_##a,
enum {
    IDATA_VID_GENERATOR(IDATA_VID_ENUM_GEN)
    IDATA_VID__NUM
};

typedef struct idata_s {
    uint32_t features;
    uint8_t mach;
    uint8_t cpu;
    uint8_t vid;
    uint8_t from_bootdisk;
    void *sfx;
    uint32_t sfxsize;
} idata_t;
#define IDATA_FEAT_SDMA         0x1     // DMA sound
#define IDATA_FEAT_ENHJOY       0x2     // Enhanced joysticks

extern idata_t idata;

#endif

