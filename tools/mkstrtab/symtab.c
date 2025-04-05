/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dtls.h"

sbuf_t symtab_sbuf;
sym_t *symtab;
static int symtabsz;
int symtab_nsyms;

int symtab_add(char *name, const sbuf_t *val)
{
    sym_t *alias = NULL;
    int i;

    if (symtab == NULL) {
        sbuf_init(&symtab_sbuf);
        symtabsz = 64;
        symtab = malloc(sizeof(sym_t) * symtabsz);
    }
    if (symtab_nsyms == symtabsz) {
        symtabsz *= 2;
        symtab = realloc(symtab, sizeof(sym_t) * symtabsz);
    }
    for (i = 0; i < symtab_nsyms; i++) {
        if (symtab[i].alias != NULL) {
            continue;
        }
        if (strcmp(symtab[i].name, name) == 0) {
            fprintf(stderr, "%s:%d: duplicate name %s\n",
                    path, lineno, name);
            return -1;
        }
        if (strcmp(symtab[i].litval, (char *)val->buf) == 0) {
            fprintf(stderr, "%s:%d: WARNING: duplicate value %s and %s\n",
                    path, lineno, name, symtab[i].name);
            alias = &symtab[i];
            break;
        }
    }
    symtab[symtab_nsyms].name = name;
    if (alias != NULL) {
        symtab[symtab_nsyms].alias = alias;
        symtab[symtab_nsyms].litval = NULL;
        symtab[symtab_nsyms].val = 0;
    } else {
        symtab[symtab_nsyms].litval = strdup((char *)val->buf);
        symtab[symtab_nsyms].val = symtab_sbuf.len;
        sbuf_append(&symtab_sbuf, val);
        symtab[symtab_nsyms].alias = NULL;
    }
    ++symtab_nsyms;
    return 0;
}

void symtab_dump(void)
{
    const sym_t *sym = symtab;
    const char *s;
    int i, c, x;

    for (i = 0; i < symtab_nsyms; i++, sym++) {
        printf("%s\n", sym->name);
        if (sym->alias != NULL) {
            printf("    alias for %s\n", sym->alias->name);
        } else {
            s = (char *)symtab_sbuf.buf + sym->val;
            x = 0;
            while ((c = *s++) != '\0') {
                if (x == 0) {
                    printf("    ");
                }
                if (c >= ' ' && c < 127) {
                    putchar(c);
                } else {
                    putchar('.');
                }
                if (++x == 60) {
                    putchar('\n');
                    x = 0;
                }
            }
            if (x != 0) {
                putchar('\n');
            }
        }
    }
}

typedef struct {
    const uint8_t *rp;
    uint16_t shiftreg;
    char nbits;
    char prev;
    char backup;
} dec64_t;

static void dec64_init(dec64_t *p, uint16_t strid)
{
    p->rp = symtab_sbuf.buf + strid;
    p->shiftreg = 0;
    p->backup = 0;
    p->nbits = 0;
}

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

static char dec64_getchar(dec64_t *p)
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
            r = symtab_sbuf.buf[cp - 1];
        } else {
            r = (cp << 6) | dec64_getcodepoint(p);
        }
    }
    p->prev = r;
    return r;
}

void symtab_dump_enc64(void)
{
    const sym_t *sym = symtab;
    dec64_t dec64;
    int i, c, x;

    for (i = 0; i < symtab_nsyms; i++, sym++) {
        printf("%s\n", sym->name);
        if (sym->alias != NULL) {
            printf("    alias for %s\n", sym->alias->name);
        } else {
            dec64_init(&dec64, sym->val);
            x = 0;
            while ((c = dec64_getchar(&dec64)) != '\0') {
                if (x == 0) {
                    printf("    ");
                }
                if (c >= ' ' && c < 127) {
                    putchar(c);
                } else {
                    putchar('.');
                }
                if (++x == 60) {
                    putchar('\n');
                    x = 0;
                }
            }
            if (x != 0) {
                putchar('\n');
            }
        }
    }
}

