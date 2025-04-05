/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _PANIC_H_
#define _PANIC_H_

void panic(uint32_t code, ...) __dead;
void error(void) __dead;

extern uint16_t panic_vector;
extern uint16_t panic_code;
extern uint16_t panic_instr;
extern uint16_t panic_sr;
extern uint32_t panic_badaddr;
extern uint32_t panic_pc;
extern uint32_t panic_aregs[8];
extern uint32_t panic_dregs[8];

void panic_handler(void) __dead;
void panic_printf(const char *fmt, ...);

#endif

