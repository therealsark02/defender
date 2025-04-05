/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Text message display.
 */

#include <stdarg.h>
#include "gd.h"
#include "sprites.h"
#include "keys.h"
#include "swtch.h"
#include "tdisp.h"
#include "font.h"
#include "mess.h"

// pre           step              return      | post
// --------  0 | ----aaaaaabb  8 | >>2 aaaaaa  | ----bb 2
// ------bb  2 | --bbbbbbcccc 10 | >>4 bbbbbb  | --cccc 4
// ----cccc  4 | ccccccdddddd 12 | >>6 cccccc  | dddddd 6
// --dddddd  6 | ------dddddd  6 | >>0 dddddd  | ------ 0
//
static uint8_t dec64_getcodepoint(dec64_t *p)
{
    uint8_t b;

    if (p->nbits < 6) {
        p->shiftreg = (p->shiftreg << 8) | *p->rp++;
        p->nbits += 8;
    }
    b = (p->shiftreg >> (p->nbits - 6)) & 0x3f;
    p->nbits -= 6;
    return b;
}

char dec64_getchar(dec64_t *p)
{
    uint8_t cp;
    char r;

    if (p->backup) {
        r = p->prev;
        p->backup = 0;
    } else {
        cp = dec64_getcodepoint(p);
        if (cp == 0) {
            r = 0;
        } else if (cp < 0x3c) {
            r = res_strtab[cp - 1];
        } else {
            r = (cp << 6) | dec64_getcodepoint(p);
        }
    }
    p->prev = r;
    return r;
}

static void dec64_backup(dec64_t *p)
{
    p->backup = 1;
}

static void charout(char c)
{
    uint8_t id;

    switch (c) {
    case '\n':
        gd->curser_y += gd->linesp; // original code spelling of 'cursor'
        break;
    case '\r':
        gd->curser_x = gd->topl_x;
        break;
    default:
        if (c != ' ') {
            if (gd->font->only_upper) {
                if (c >= 'a' && c <= 'z') {
                    c &= ~0x20;
                }
                if (c >= '!' && c <= '`') {
                    id = c - ' ';
                } else if (c >= '{' && c <= '~') {
                    id = c - '{' + 0x41;
                } else {
                    id = '?' - ' ';
                }
            } else {
                id = c - ' ';
            }
            render_char(gd->curser_x, gd->curser_y, &gd->font->glyphs[id * 8]);
            if (gd->font->widths != NULL) {
                gd->curser_x += gd->font->widths[id] + gd->charsp;
            } else {
                gd->curser_x += gd->font->fixed_width + gd->charsp;
            }
        } else {
            gd->curser_x += 4;
        }
        break;
    }
}

static void strout(const char *s)
{
    char c;

    while ((c = *s++) != '\0') {
        charout(c);
    }
}

static void rstrout(int strid)
{
    static dec64_t dec64;
    char c;

    dec64_init(&dec64, strid);
    while ((c = dec64_getchar(&dec64)) != '\0') {
        charout(c);
    }
}

static void bcdout(uint16_t n)
{
    uint8_t z = 1;
    uint8_t i, d;

    for (i = 0; i < 4; i++) {
        d = (n >> ((3 - i) * 4)) & 0xf;
        if (d || !z) {
            charout('0' + d);
            z = 0;
        }
    }
    if (z) {
        charout('0');
    }
}

static void intout(int n)
{
    static char opbuf[10];
    uint32_t in;
    char *p;

    if (n == 0) {
        charout('0');
        return;
    }
    in = n;
    if (n < 0) {
        charout('-');
        in = -n;
    }
    p = opbuf;
    while (in) {
        *p++ = '0' + (in % 10U);
        in /= 10;
    }
    do {
        charout(*--p);
    } while (p > opbuf);
}

static void scoreout(uint32_t n)
{
    uint8_t i, d, z;

    z = 0;
    for (i = 0; i < 6; i++) {
        d = (n >> 20) & 0xf;
        n <<= 4;
        if (d == 0 && !z) {
            gd->curser_x += 8;
            continue;
        }
        charout('0' + d);
        if (d != 0) {
            z = 1;
        }
    }
}

static void keynameout(uint32_t k)
{
    const char *s = "???";

    if (k < KEY__NUM) {
        if (keynames[k] != NULL) {
            s = keynames[k];
        }
    }
    strout(s);
}

static void outnum(uint32_t n, uint8_t ndigs)
{
    uint8_t d, i;

    for (i = 0; i < ndigs; i++) {
        d = (n >> (4 * (ndigs - 1 - i))) & 0xf;
        if (d <= 9) {
            charout('0' + d);
        } else {
            charout('A' + d - 10);
        }
    }
}

void messf(uint16_t x, uint8_t y, uint16_t strid, ...)
{
    static dec64_t dec64;
    va_list ap;
    char c;

    gd->curser_x = gd->topl_x = x;
    gd->curser_y = gd->topl_y = y;
    gd->charsp = gd->font->charsp;
    gd->linesp = 10;

    dec64_init(&dec64, strid);
    va_start(ap, strid);
    while ((c = dec64_getchar(&dec64)) != '\0') {
        if (c == '%') {
            c = dec64_getchar(&dec64);
            switch (c) {
            case '\0':
                dec64_backup(&dec64);
                break;
            case '%':
                charout(c);
                break;
            case 'd':
                intout(va_arg(ap, int));
                break;
            case 'c':
                charout(va_arg(ap, int));
                break;
            case 's':
                strout(va_arg(ap, char *));
                break;
            case 'r':
                rstrout(va_arg(ap, int));
                break;
            case 'b':
                bcdout(va_arg(ap, uint32_t));
                break;
            case 'S':
                scoreout(va_arg(ap, uint32_t));
                break;
            case 'k':
                keynameout(va_arg(ap, uint32_t));
                break;
            case 'B':
                outnum(va_arg(ap, uint32_t), 2);
                break;
            case 'W':
                outnum(va_arg(ap, uint32_t), 4);
                break;
            case 'L':
                outnum(va_arg(ap, uint32_t), 8);
                break;
            default:
                charout('?');
                break;
            }
        } else if (c == '\e') {
            c = dec64_getchar(&dec64);
            if (c == '\0') {
                dec64_backup(&dec64);
                break;
            }
            switch (c) {
            case 'M':
                c = dec64_getchar(&dec64);
                if (c == '\0') {
                    dec64_backup(&dec64);
                    break;
                }
                gd->curser_x += (uint8_t)c;
                break;
            case 'H':
                c = dec64_getchar(&dec64);
                if (c == '\0') {
                    dec64_backup(&dec64);
                    break;
                }
                gd->curser_x = gd->topl_x + (uint8_t)c;
                break;
            case 'V':
                c = dec64_getchar(&dec64);
                if (c == '\0') {
                    dec64_backup(&dec64);
                    break;
                }
                gd->curser_y = gd->topl_y + (uint8_t)c;
                break;
            default:
                break;
            }
        } else {
            charout(c);
        }
    }
    va_end(ap);
}

