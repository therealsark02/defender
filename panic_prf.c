/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Panic text output.
 */

#include <stdarg.h>
#include "gd.h"
#include "color.h"
#include "font.h"
#include "panic.h"

static void charout(char c)
{
    uint16_t x = 8 * gd->panic_col;
    uint8_t y = 7 + gd->panic_row * 8;
    uint8_t id;

    if (c == '\n') {
        gd->panic_col = 0;
        ++gd->panic_row;
        return;
    }
    if (c != ' ') {
        if (c >= '!' && c <= '_') {
            id = c - ' ';
        } else if (c == '`') {
            id = 0;
        } else {
            id = '?' - ' ';
        }
        render_char(x, y, &atari_font.glyphs[id * 8]);
    }
    ++gd->panic_col;
}

static void u4out(uint8_t d)
{
    d &= 0xf;
    if (d <= 9) {
        charout('0' + d);
    } else {
        charout('A' + d - 10);
    }
}

static void u8out(uint8_t n)
{
    u4out(n >> 4);
    u4out(n);
}

static void u16out(uint16_t n)
{
    u8out(n >> 8);
    u8out(n);
}

static void u24out(uint32_t n)
{
    u8out(n >> 16);
    u16out(n);
}

static void u32out(uint32_t n)
{
    u16out(n >> 16);
    u16out(n);
}

static void strout(char k, const char *s)
{
    char c;

    if (s == NULL) {
        strout('s', "<null>");
    } else {
        while ((c = *s++) != '\0') {
            c = (c == '_' && k == 'S') ? ' ' : c;
            charout(c);
        }
    }
}

void panic_printf(const char *fmt, ...)
{
    va_list ap;
    char c;

    va_start(ap, fmt);
    while ((c = *fmt++) != '\0') {
        if (c == '%') {
            c = *fmt++;
            switch (c) {
            case '\0':
                --fmt;
                break;
            case '%':
                charout(c);
                break;
            case 'c':
                charout(va_arg(ap, uint32_t));
                break;
            case 'n':
                u4out(va_arg(ap, uint32_t));
                break;
            case 'b':
                u8out(va_arg(ap, uint32_t));
                break;
            case 'w':
                u16out(va_arg(ap, uint32_t));
                break;
            case 'a':
                u24out(va_arg(ap, uint32_t));
                break;
            case 'l':
                u32out(va_arg(ap, uint32_t));
                break;
            case 's':
            case 'S':
                strout(c, va_arg(ap, char *));
                break;
            default:
                charout('?');
                break;
            }
        } else {
            charout(c);
        }
    }
    va_end(ap);
}

