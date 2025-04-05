/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Switch / button / keyboard handling.
 */

#include "gd.h"
#include "plstrt.h"
#include "player.h"
#include "laser.h"
#include "sbomb.h"
#include "hyper.h"
#include "sound.h"
#include "setup.h"
#include "attract.h"
#include "sprites.h"
#include "joypad.h"
#include "tdisp.h"
#include "mfp.h"
#include "vectors.h"
#include "keys.h"
#include "acia.h"
#include "irq.h"
#include "cmos.h"
#include "swtch.h"

#define KEY_STR_GEN(a, b, c) [b] = c,
const char *keynames[KEY__NUM] = {
    KEY_GENERATOR(KEY_STR_GEN)
};

static const swdata_t swtab[8] = {
    { lfire, STYPE, 0xe8 }, // fire
    { 0, 0, 0 },            // thrust
    { sbomb, STYPE, 0xf8 }, // smart bomb
    { hyper, STYPE, 0xf8 },
    { st2, STYPE, 0 },
    { st1, STYPE, 0 },
    { rev, STYPE, 0xe8 },
    { 0, 0, 0 },
};

static void lcoin(void) { sucide(); }
static void hsreset(void) { sucide(); }
static void ccoin(void) { sucide(); }

static const swdata_t swtab1[8] = { 
    { 0, 0, 0 },            // AUTOUP
    { setupsw, STYPE, 0 },  // ADVANCE F4 to setup
    { helpsw, STYPE, 0 },   // RCOIN   F3 to help
    { hsreset, STYPE, 0 },  // HSRESET F5
    { lcoin, CTYPE, 0 },    // LCOIN
    { ccoin, CTYPE, 0 },    // CCOIN
    { 0, 0, 0 },
    { 0, 0, 0 },
};

swdata_t swproc[2];

static uint8_t kbuf[KBUFSZ];   // key buffer

static void swtch(const swdata_t *swp, uint8_t swbits)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        if (swbits & (1 << i)) {
            if (swproc[0].paddr == NULL) {
                swproc[0] = swp[i];
            } else {
                swproc[1] = swp[i];
            }
            return;
        }
    }
    panic(0x8e338f12, swp, swbits);
}

static const uint8_t kargs[] = {
    7,  // 1111_0110 F6   : +7 bytes: sync various
    5,  // 1111_0111 F7   : +5 bytes: async absolute mouse report
    2,  // 1111_10xx F8-FB: +2 bytes: async relative mouse report
    2,  // 1111_10xx F8-FB: +2 bytes: async relative mouse report
    2,  // 1111_10xx F8-FB: +2 bytes: async relative mouse report
    2,  // 1111_10xx F8-FB: +2 bytes: async relative mouse report
    6,  // 1111_1100 FC   : +6 bytes: sync clock
    2,  // 1111_1101 FD   : +2 bytes: continuous joystick report(?)
    1,  // 1111_111x FE   : +1 byte:  async joystick 0 report
    1,  // 1111_111x FF   : +1 byte:  async joystick 1 report
};

void keyscan(keyfunc_t func)
{
    uint32_t jpad, jupbits, jdnbits;
    uint32_t mask, jpjoymask;
    uint16_t flags;
    uint8_t d, i;
    
    flags = irq_disable();
    while (gd->kci != gd->kpi) {
        d = kbuf[gd->kci];
        gd->kci = (gd->kci + 1) % KBUFSZ;
        irq_restore(flags);
        if (gd->kst == 0) {
            if (d >= 0xf6) {
                gd->kcmd = d;
                gd->kargc = kargs[d - 0xf6];
                gd->kidx = 0;
                gd->kst = 1;
            } else {
                // simple key report
                func(KEY_EVENT(d & 0x80, d & 0x7f));
            }
        } else {
            gd->kargs[gd->kidx++] = d;
            if (gd->kidx == gd->kargc) {
                switch (gd->kcmd) {
                case 0xff:
                    // joystick 1
                    d = gd->kargs[0];
                    func(JOY_EVENT(d));
                    break;
                default:
                    // ignore others
                    break;
                }
                gd->kst = 0;
            }
        }
        flags = irq_disable();
    }
    irq_restore(flags);

    if (cfg.controller == CFG_CONTROLLER_JOYPAD) {
        jpad = joypad_read();
        if ((gd->jpad ^ jpad) != 0) {
            jpjoymask = JOYPAD_LEFT_BIT | JOYPAD_RIGHT_BIT |
                        JOYPAD_UP_BIT | JOYPAD_DOWN_BIT;
            jdnbits = (~gd->jpad & jpad) & ~jpjoymask;
            jupbits = (gd->jpad & ~jpad) & ~jpjoymask;
            mask = 1;
            for (i = 0; i < JOYPAD__NUM; i++, mask <<= 1) {
                if (jupbits & mask) {
                    func(KEY_EVENT(1, KEY_JPAD_PAUSE + i));
                } else if (jdnbits & mask) {
                    func(KEY_EVENT(0, KEY_JPAD_PAUSE + i));
                }
            }
            if ((gd->jpad ^ jpad) & jpjoymask) {
                func(JOYPAD_EVENT(jpad));
            }
        }
        gd->jpad = jpad;
    }
}

static void cscan(void)
{
    uint8_t swbits;

    swbits = ~gd->pia02 & gd->pia01;
    gd->pia02 = gd->pia01;
    if (swbits) {
        swtch(swtab1, swbits);
    }
}

static void keyhandler(uint16_t event)
{
    const uint8_t *kb = cfg.kbindings[cfg.controller];
    uint8_t d;
    
    if (EV_TYPE(event) == EV_KEYBOARD) {
        d = EV_KEY(event);
        if (EV_KEYUP(event)) {
            if (d == kb[KB_UP]) {
                gd->pia31 &= ~PIA31_UP;
            } else if (d == kb[KB_DOWN]) {
                gd->pia21 &= ~PIA21_DOWN;
            } else if (d == kb[KB_FIRE]) {
                gd->pia21 &= ~PIA21_FIRE;
            } else if (d == kb[KB_THRUST]) {
                gd->pia21 &= ~PIA21_THRUST;
            } else if (d == kb[KB_SMARTBOMB]) {
                gd->pia21 &= ~PIA21_SMARTBOMB;
            } else if (d == kb[KB_HYPERSPACE]) {
                gd->pia21 &= ~PIA21_HYPERSPACE;
            } else if (d == kb[KB_REVERSE]) {
                gd->pia21 &= ~PIA21_REVERSE;
            } else if (d == KEY_F1) {
                gd->pia21 &= ~PIA21_1PLAYER;
            } else if (d == KEY_F2) {
                gd->pia21 &= ~PIA21_2PLAYERS;
            } else if (d == KEY_F3) {
                gd->pia01 &= ~PIA01_RCOIN;
            } else if (d == KEY_F4) {
                gd->pia01 &= ~PIA01_ADVANCE;
            } else if (d == KEY_F5) {
                gd->pia01 &= ~PIA01_HSRESET;
            }
        } else {
            if (d == kb[KB_UP]) {
                gd->pia31 |= PIA31_UP;
                gd->pia21 &= ~PIA21_DOWN;
            } else if (d == kb[KB_DOWN]) {
                gd->pia21 |= PIA21_DOWN;
                gd->pia31 &= ~PIA31_UP;
            } else if (d == kb[KB_FIRE]) {
                gd->pia21 |= PIA21_FIRE;
            } else if (d == kb[KB_THRUST]) {
                gd->pia21 |= PIA21_THRUST;
            } else if (d == kb[KB_SMARTBOMB]) {
                gd->pia21 |= PIA21_SMARTBOMB;
            } else if (d == kb[KB_HYPERSPACE]) {
                gd->pia21 |= PIA21_HYPERSPACE;
            } else if (d == kb[KB_REVERSE]) {
                gd->pia21 |= PIA21_REVERSE;
            } else if (d == KEY_F1) {
                gd->pia21 |= PIA21_1PLAYER;
            } else if (d == KEY_F2) {
                gd->pia21 |= PIA21_2PLAYERS;
            } else if (d == KEY_F3) {
                gd->pia01 |= PIA01_RCOIN;
            } else if (d == KEY_F4) {
                gd->pia01 |= PIA01_ADVANCE;
            } else if (d == KEY_F5) {
                gd->pia01 |= PIA01_HSRESET;
            } else if (d == KEY_F6 && (gd->cheats & CHEAT_SCORE)) {
                gd->killme = 1;
            } else if (d == KEY_F7 && (gd->cheats & CHEAT_SCORE)) {
                score(0x301);
            } else if (d == KEY_F8) {
                gd->snddis ^= 1;
                if (gd->snddis) {
                    sndout(0x13);
                }
            } else if (d == KEY_F9) {
                gd->want_grid ^= 1;
            } else if (d == KEY_F10) {
                gd->want_timing ^= 1;
            }
        }
    } else if (EV_TYPE(event) == EV_JOYSTICK &&
               cfg.controller == CFG_CONTROLLER_JOYSTICK) {
        d = EV_JOY(event);
        gd->joy1 = d;
        gd->pia21 &= ~(PIA21_DOWN | PIA21_THRUST | PIA21_FIRE);
        gd->pia31 &= ~PIA31_UP;
        if (d & JOY_FIRE) {
            gd->pia21 |= PIA21_FIRE;
        }
        if (d & JOY_DOWN) {
            gd->pia21 |= PIA21_DOWN;
        }
        if (d & JOY_UP) {
            gd->pia31 |= PIA31_UP;
        }
        if (d & (JOY_LEFT | JOY_RIGHT)) {
            // REVERSE is handled by swp() in exec.c
            // setting it here does not work, gets
            // stuck on, and other problems.
            gd->pia21 |= PIA21_THRUST;
        }
    } else if (EV_TYPE(event) == EV_JOYPAD &&
               cfg.controller == CFG_CONTROLLER_JOYPAD) {
        d = EV_JPAD(event);
        gd->pia21 &= ~(PIA21_DOWN | PIA21_THRUST);
        gd->pia31 &= ~PIA31_UP;
        if (d & EV_JPAD_DOWN) {
            gd->pia21 |= PIA21_DOWN;
        }
        if (d & EV_JPAD_UP) {
            gd->pia31 |= PIA31_UP;
        }
        if (d & (EV_JPAD_LEFT | EV_JPAD_RIGHT)) {
            // REVERSE is handled by swp() in exec.c
            // setting it here does not work, gets
            // stuck on, and other problems.
            gd->pia21 |= PIA21_THRUST;
        }
    }
}

// switch scan
void sscan(void)
{
    uint8_t swbits;

    gd->pia22 = gd->pia21;
    keyscan(keyhandler);
    swbits = ~gd->pia22 & gd->pia21;
    if (swbits) {
        swtch(swtab, swbits);
    } else {
        cscan();
    }
}

#define ACIA_RX_INTS    (ACIA_STAT_RXFULL | \
                         ACIA_STAT_EFRM | \
                         ACIA_STAT_ERXOVR | \
                         ACIA_STAT_EPARITY)
void key_irq_handler(void)
{
    uint8_t stat, data;
    gd_t *gd = &g_gd;

    stat = readb(ACIA_KEYB_STAT);
    while (stat & ACIA_RX_INTS) {
        data = readb(ACIA_KEYB_DATA);
        if (!(stat & (ACIA_STAT_EFRM | ACIA_STAT_EPARITY))) {
            // good data
            kbuf[gd->kpi] = data;
            gd->kpi = (gd->kpi + 1) % KBUFSZ;
            if (gd->kpi == gd->kci) {
                panic(0xdeadca00);
            }
        }
        stat = readb(ACIA_KEYB_STAT);
    }
}

static void acia_send(uint8_t cmd)
{
    while (!(readb(ACIA_KEYB_STAT) & ACIA_STAT_TXEMPTY)) {
        /* spin */
    }
    writeb(ACIA_KEYB_DATA, cmd);
}

void keyboard_init(void)
{
    // reset midi side
    writeb(ACIA_MIDI_CTRL, ACIA_CTRL_MST_RESET);

    // reset the keyboard side and re-enable
    writeb(ACIA_KEYB_CTRL, ACIA_CTRL_MST_RESET);
    writeb(ACIA_KEYB_CTRL, ACIA_CTRL_RXIE | ACIA_CTRL_8N1 | ACIA_CTRL_DIV_64);

    // disable mouse reports
    acia_send(0x12);

    // vector the keyboard
    vector_set(V_IKBD, hwkeyb);

    // enable keyboard irq
    mfp_ierb_set(1 << 6);
    mfp_imrb_set(1 << 6);

    // clear any pending interrupt condition
    readb(ACIA_KEYB_STAT);
    readb(ACIA_KEYB_DATA);
}

void keyboard_os_restore(void)
{
    // reset the keyboard side and re-enable
    writeb(ACIA_KEYB_CTRL, ACIA_CTRL_MST_RESET);
    writeb(ACIA_KEYB_CTRL, ACIA_CTRL_RXIE | ACIA_CTRL_8N1 | ACIA_CTRL_DIV_64);

    // send mouse reports
    acia_send(0x08);

    // clear any pending interrupt condition
    readb(ACIA_KEYB_STAT);
    readb(ACIA_KEYB_DATA);
}

