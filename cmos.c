/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Persistent storage functions.
 */

#include "gd.h"
#include "data.h"
#include "keys.h"
#include "swtch.h"
#include "sys.h"
#include "cmos.h"

static const cfg_t default_cfg = {
    .kbindings = {
        [CFG_CONTROLLER_KEYBOARD] = {
            [KB_UP] = KEY_D,
            [KB_DOWN] = KEY_C,
            [KB_THRUST] = KEY_M,
            [KB_REVERSE] = KEY_SPACE,
            [KB_FIRE] = KEY_L,
            [KB_SMARTBOMB] = KEY_S,
            [KB_HYPERSPACE] = KEY_F
        },
        [CFG_CONTROLLER_JOYSTICK] = {
            [KB_SMARTBOMB] = KEY_SPACE,
            [KB_HYPERSPACE] = KEY_LSHIFT
        },
        [CFG_CONTROLLER_JOYPAD] = {
            [KB_FIRE] = KEY_JPAD_B,
            [KB_SMARTBOMB] = KEY_JPAD_C,
            [KB_HYPERSPACE] = KEY_JPAD_A
        },
    },
    .autosave_hst = 0,
    .controller = CFG_CONTROLLER_KEYBOARD,
    .setup_font = CFG_FONT_DEFENDER,
};

static const cmosdata_t defalt = {
    {
        { { 0x02, 0x12, 0x70 }, "DRJ" },
        { { 0x01, 0x83, 0x15 }, "SAM" },
        { { 0x01, 0x59, 0x20 }, "LED" },
        { { 0x01, 0x42, 0x85 }, "PGD" },
        { { 0x01, 0x25, 0x20 }, "CRB" },
        { { 0x01, 0x10, 0x35 }, "MRS" },
        { { 0x00, 0x82, 0x65 }, "SSR" },
        { { 0x00, 0x60, 0x10 }, "TMH" }
    },
    0x00,           // dipsw
    0x5a,           // cmosck
    0x1000,         // replay @10,000
    0x03,           // nship
    0x03,           // cselct
    0x01,           // slot1m
    0x04,           // slot2m
    0x01,           // slot3m
    0x01,           // cunitc
    0x00,           // cunitb
    0x00,           // minunt
    0x00,           // freepl
    0x05,           // ga1  initial difficulty
    0x15,           // ga2  difficulty ceiling
    0x01,           // ga3  unused
    0x05,           // ga4  restore wave #
    0x00,           // ga5
    0x00,           // ga6
    0x00,           // ga7
    0x00,           // ga8
    0x00,           // ga9
    0x00,           // ga10
};

cmosdata_t cmsdef;
cfg_t cfg;
hofent_t todays_hst[8];
hofent_t alltime_hst[8];

void reset_kbindings(void)
{
    // set default key bindings for current controller
    memcpy(&cfg.kbindings[cfg.controller], &default_cfg.kbindings[cfg.controller], sizeof(cfg.kbindings[0]));
}

static void reset_all_kbindings(void)
{
    // set default key bindings
    memcpy(&cfg.kbindings, &default_cfg.kbindings, sizeof(cfg.kbindings));
    cfg.controller = CFG_CONTROLLER_KEYBOARD;
}

void reset_all_config(void)
{
    memcpy(&cfg, &default_cfg, sizeof(cfg));
}

void reset_todays_hst(void)
{
    memcpy(todays_hst, cmsdef.hst, sizeof(todays_hst));
}

void reset_all_hst(void)
{
    memcpy(alltime_hst, cmsdef.hst, sizeof(alltime_hst));
    reset_todays_hst();
}

static const uint8_t cntrl_usedmask[CFG_CONTROLLER__NUM] = {
    [CFG_CONTROLLER_KEYBOARD] =
        (1 << KB_UP)  | (1 << KB_DOWN) | (1 << KB_THRUST) | (1 << KB_REVERSE) |
        (1 << KB_FIRE) | (1 << KB_SMARTBOMB) | (1 << KB_HYPERSPACE),
    [CFG_CONTROLLER_JOYSTICK] = 
        (1 << KB_SMARTBOMB) | (1 << KB_HYPERSPACE),
    [CFG_CONTROLLER_JOYPAD] =
        (1 << KB_FIRE) | (1 << KB_SMARTBOMB) | (1 << KB_HYPERSPACE),
};

static int is_valid_kbindings(void)
{
    uint8_t usedmask;
    int i, j, kn;

    if ((gd->has_joypad && cfg.controller > CFG_CONTROLLER_JOYPAD) ||
        (!gd->has_joypad && cfg.controller > CFG_CONTROLLER_JOYSTICK)) {
        return 0;
    }
    for (i = 0; i < CFG_CONTROLLER__NUM; i++) {
        usedmask = cntrl_usedmask[i];
        for (j = 0; j < KB__NUM; j++) {
            if (usedmask & (1 << j)) {
                kn = cfg.kbindings[i][j];
                if (kn >= KEY__NUM || keynames[kn] == NULL) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static void sanitize_cfg(void)
{
    if (!is_valid_kbindings()) {
        reset_all_kbindings();
    }
    if (cfg.setup_font >= CFG_FONT__NUM) {
        cfg.setup_font = CFG_FONT_DEFENDER;
    }
}

static uint16_t file_sum(const void *buf, int len)
{
    const uint8_t *bp = buf;
    uint16_t sum = 0;

    while (len--) {
        sum = (sum << 4) ^ (sum >> 12) ^ *bp++;
    }
    return sum;
}

static int is_cfgbuf_valid(cfg_data_t *buf)
{
    uint16_t sum;

    if (buf->magic != CFG_MAGIC || buf->version != CFG_VERSION ||
        buf->len != sizeof(*buf)) {
        return 0;
    }
    sum = buf->sum;
    buf->sum = 0;
    return (sum == file_sum(buf, buf->len));
}

static int is_hstbuf_valid(hst_data_t *buf)
{
    uint16_t sum;

    if (buf->magic != HST_MAGIC || buf->version != HST_VERSION ||
        buf->len != sizeof(*buf)) {
        return 0;
    }
    sum = buf->sum;
    buf->sum = 0;
    return (sum == file_sum(buf, buf->len));
}

static void cfg_init(void)
{
    cfg_data_t *src = (void *)cfg_buf;

    if (is_cfgbuf_valid(src)) {
        memcpy(&cfg, &src->cfg, sizeof(cfg));
        sanitize_cfg();
    } else {
        reset_all_config();
    }
}

static void hst_init(void)
{
    hst_data_t *src = (void *)hst_buf;

    if (is_hstbuf_valid(src)) {
        memcpy(alltime_hst, &src->alltime_hst, sizeof(alltime_hst));
        reset_todays_hst();
    } else {
        reset_all_hst();
    }
}

void cminit(void)
{
    memcpy(&cmsdef, &defalt, sizeof(cmsdef));
    cfg_init();
    hst_init();
}

int write_cfg(void)
{
    cfg_data_t *buf = (void *)cfg_buf;

    cfg_len = sizeof(*buf);
    memset(buf, 0, cfg_len);
    buf->magic = CFG_MAGIC;
    buf->version = CFG_VERSION;
    buf->len = cfg_len;
    memcpy(&buf->cfg, &cfg, sizeof(buf->cfg));
    buf->sum = file_sum(buf, cfg_len);
    return save_cfg_file();
}

int read_cfg(void)
{
    cfg_data_t *buf = (void *)cfg_buf;

    if (load_cfg_file() < 0) {
        return -1;
    }
    if (is_cfgbuf_valid(buf)) {
        memcpy(&cfg, &buf->cfg, sizeof(cfg));
        sanitize_cfg();
        return 0;
    } else {
        gd->file_err = FILE_ERR_INVALID;
        return -1;
    }
}

int write_hst(void)
{
    hst_data_t *buf = (void *)hst_buf;

    hst_len = sizeof(*buf);
    memset(buf, 0, hst_len);
    buf->magic = HST_MAGIC;
    buf->version = HST_VERSION;
    buf->len = hst_len;
    memcpy(buf->alltime_hst, alltime_hst, sizeof(buf->alltime_hst));
    buf->sum = file_sum(buf, hst_len);
    return save_hst_file();
}

int read_hst(void)
{
    hst_data_t *buf = (void *)hst_buf;

    if (load_hst_file() < 0) {
        return -1;
    }
    if (is_hstbuf_valid(buf)) {
        memcpy(alltime_hst, &buf->alltime_hst, sizeof(alltime_hst));
        return 0;
    } else {
        gd->file_err = FILE_ERR_INVALID;
        return -1;
    }
}

