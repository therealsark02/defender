/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SETUP_H_
#define _SETUP_H_

void setupsw(void) __dead;
void enter_setup(void) __dead;
void file_wrapper(int strid, int (*func)(void));
void setup_show_keys(uint8_t y);
void setup_report(int strid);
void setup_title(int strid);
uint8_t setup_getnumkey(uint8_t top);

#endif

