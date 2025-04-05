/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _DTLS_H_
#define _DTLS_H_

typedef struct {
    uint8_t *buf;
    int len;
    int size;
} sbuf_t;

void sbuf_init(sbuf_t *p);
void sbuf_strcat(sbuf_t *p, const char *s);
void sbuf_append(sbuf_t *p, const sbuf_t *q);
void sbuf_push(sbuf_t *p, uint8_t b);

typedef struct sym_s {
    const char *name;
    const char *litval;
    struct sym_s *alias;
    uint16_t val;
} sym_t;

extern sbuf_t symtab_sbuf;
extern sym_t *symtab;
extern int symtab_nsyms;
extern const char *path;
extern int lineno;

int symtab_add(char *name, const sbuf_t *val);
void symtab_dump(void);
void symtab_dump_enc64(void);

#endif

