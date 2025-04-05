/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _ACIA_H_
#define _ACIA_H_

// ACIA Registers
#define ACIA_KEYB_CTRL      0xfffffc00
#define ACIA_KEYB_STAT      0xfffffc00
#define ACIA_KEYB_DATA      0xfffffc02
#define ACIA_MIDI_CTRL      0xfffffc04
#define ACIA_MIDI_STAT      0xfffffc04
#define ACIA_MIDI_DATA      0xfffffc06

#define ACIA_CTRL_DIV_NORML 0x00
#define ACIA_CTRL_DIV_16    0x01
#define ACIA_CTRL_DIV_64    0x02
#define ACIA_CTRL_MST_RESET 0x03
#define ACIA_CTRL_7E2       0x00
#define ACIA_CTRL_7O2       0x04
#define ACIA_CTRL_7E1       0x0c
#define ACIA_CTRL_7O1       0x0c
#define ACIA_CTRL_8N2       0x10
#define ACIA_CTRL_8N1       0x14
#define ACIA_CTRL_8E1       0x1c
#define ACIA_CTRL_8O1       0x1c
#define ACIA_CTRL_RTSLTXIE  0x20
#define ACIA_CTRL_RTSHTXID  0x40
#define ACIA_CTRL_RTSLBTXID 0x60
#define ACIA_CTRL_RXIE      0x80

#define ACIA_STAT_RXFULL    0x01
#define ACIA_STAT_TXEMPTY   0x02
#define ACIA_STAT_DCD       0x04
#define ACIA_STAT_CTS       0x08
#define ACIA_STAT_EFRM      0x10
#define ACIA_STAT_ERXOVR    0x20
#define ACIA_STAT_EPARITY   0x40
#define ACIA_STAT_INTREQ    0x80

#endif

