/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _CMOS_H_
#define _CMOS_H_

typedef struct hofent_s {
    uint8_t score[3];
    char name[3];
} hofent_t;

typedef struct {
    hofent_t hst[8];
    uint8_t    dipsw;
    uint8_t    cmosck;
    uint16_t   replay;
    uint8_t    nship;
    uint8_t    cselct;
    uint8_t    slot1m;
    uint8_t    slot2m;
    uint8_t    slot3m;
    uint8_t    cunitc;
    uint8_t    cunitb;
    uint8_t    minunt;
    uint8_t    freepl;
    uint8_t    ga1;     // initial difficulty
    uint8_t    ga2;     // difficulty ceiling
    uint8_t    ga3;     // unused
    uint8_t    ga4;     // restore wave #
    uint8_t    ga5;
    uint8_t    ga6;
    uint8_t    ga7;
    uint8_t    ga8;
    uint8_t    ga9;
    uint8_t    ga10;
} cmosdata_t;

typedef struct {
    uint16_t magic;
    uint16_t version;
    uint16_t len;
    uint16_t sum;
    hofent_t alltime_hst[8];
} hst_data_t;
#define HST_MAGIC   0x4e48
#define HST_VERSION 1

#define CFG_CONTROLLER_GENERATOR(mac) \
    mac(KEYBOARD) \
    mac(JOYSTICK) \
    mac(JOYPAD)

#define CFG_CONTROLLER_ENUM_GEN(a) CFG_CONTROLLER_##a,
enum {
    CFG_CONTROLLER_GENERATOR(CFG_CONTROLLER_ENUM_GEN)
    CFG_CONTROLLER__NUM,
};

#define KBINDING_GENERATOR(mac) \
    mac(UP) \
    mac(DOWN) \
    mac(THRUST) \
    mac(REVERSE) \
    mac(FIRE) \
    mac(SMARTBOMB) \
    mac(HYPERSPACE)

#define KBINDING_ENUM_GEN(a) KB_##a,
enum {
    KBINDING_GENERATOR(KBINDING_ENUM_GEN)
    KB__NUM,
};

#define CFG_FONT_GENERATOR(mac) \
    mac(DEFENDER) \
    mac(ATARI)

#define CFG_FONT_ENUM_GEN(a) CFG_FONT_##a,
enum {
    CFG_FONT_GENERATOR(CFG_FONT_ENUM_GEN)
    CFG_FONT__NUM,
};

typedef struct {
    uint8_t kbindings[CFG_CONTROLLER__NUM][KB__NUM];
    uint8_t autosave_hst;
    uint8_t controller;
    uint8_t setup_font;
} cfg_t;
extern cfg_t cfg;

typedef struct {
    uint16_t magic;
    uint16_t version;
    uint16_t len;
    uint16_t sum;
    cfg_t cfg;
} cfg_data_t;
#define CFG_MAGIC   0x270f
#define CFG_VERSION 1

int write_cfg(void);
int read_cfg(void);
int write_hst(void);
int read_hst(void);

void reset_kbindings(void);
void reset_all_config(void);

extern hofent_t todays_hst[8];
extern hofent_t alltime_hst[8];

void cminit(void);
void reset_todays_hst(void);
void reset_all_hst(void);

#endif

