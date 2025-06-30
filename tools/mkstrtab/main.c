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

const char *path;
int lineno;

static int normalize_buf(sbuf_t *sbuf)
{
    uint8_t *rp = sbuf->buf;
    uint8_t *wp = sbuf->buf;
    int c, base, cnt, val = 0;
    int state = 0;
    
    // compile the string
    while ((c = *rp++) != '\0') {
        switch (state) {
        case 0:
            if (c == '\\') {
                state = 1;
            } else {
                *wp++ = c;
            }
            break;
        case 1:
            switch (c) {
            case '\\':
                *wp++ = '\\';
                state = 0;
                break;
            case '"':
                *wp++ = '"';
                state = 0;
                break;
            case 'e':
                *wp++ = '\e';
                state = 0;
                break;
            case 'r':
                *wp++ = '\r';
                state = 0;
                break;
            case 'n':
                *wp++ = '\n';
                state = 0;
                break;
            case 'x':
                val = 0;
                state = 2;
                base = 16;
                cnt = 2;
                break;
            case '0'...'7':
                val = c - '0';
                state = 2;
                base = 8;
                cnt = 2;
                break;
            default:
                fprintf(stderr, "unknown escape \\%c\n", c);
                return -1;
            }
            break;
        case 2:
            if (base == 8) {
                if (c >= '0' && c <= '7') {
                    val = (val << 3) + c - '0';
                } else {
                    fprintf(stderr, "invalid octal escape\n");
                    return -1;
                }
            } else {
                if (c >= '0' && c <= '9') {
                    val = (val << 4) + c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    val = (val << 4) + c - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    val = (val << 4) + c - 'A' + 10;
                } else {
                    fprintf(stderr, "invalid hex escape\n");
                    return -1;
                }
            }
            if (--cnt == 0) {
                *wp++ = val;
                state = 0;
            }
            break;
        }
    }
    *wp++ = '\0';
    sbuf->len = wp - sbuf->buf;
    return 0;
}

static int defn_finish(char *symname, sbuf_t *sbuf)
{
    if (normalize_buf(sbuf) < 0) {
        fprintf(stderr, "%s:%d: invalid string\n", path, lineno);
        return -1;
    }
    return symtab_add(symname, sbuf);
}

static int read_resfile(const char *_path)
{
    char buf[256];
    char *symname;
    int llen, i, lmargin, rmargin;
    sbuf_t sbuf = { 0 };
    FILE *fp;

    path = _path;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    symname = NULL;
    lineno = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        ++lineno;
        if (memcmp(buf, "ID_", 3) == 0) {
            if (symname != NULL) {
                if (defn_finish(symname, &sbuf) < 0) {
                    goto err;
                }
            }
            symname = strdup(strtok(buf, " \t\n"));
            sbuf_init(&sbuf);
        } else {
            llen = strlen(buf);
            lmargin = 99;
            rmargin = -1;
            for (i = 0; i < llen; i++) {
                if (buf[i] == '"') {
                    if (i < lmargin) {
                        lmargin = i;
                    } else if (i > rmargin) {
                        rmargin = i;
                    }
                }
            }
            if (lmargin == 99) {
                if (symname != NULL) {
                    if (defn_finish(symname, &sbuf) < 0) {
                        goto err;
                    }
                    symname = NULL;
                }
            } else {
                if (lmargin >= rmargin) {
                    fprintf(stderr, "%s:%d: invalid format\n", path, lineno);
                    goto err;
                }
                buf[rmargin] = '\0';
                sbuf_strcat(&sbuf, buf + lmargin + 1);
            }
        }
    }
    if (symname != NULL) {
        if (defn_finish(symname, &sbuf) < 0) {
            goto err;
        }
    }
    fclose(fp);
    return 0;
err:
    fclose(fp);
    return -1;
}

static int write_hfile(const char *path)
{
    const sym_t *sym;
    int i, val;
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    fprintf(fp, "\n#ifndef _RESOURCES_H_\n#define _RESOURCES_H_\n\n");
    sym = symtab;
    for (i = 0; i < symtab_nsyms; i++, sym++) {
        val = (sym->alias == NULL) ? sym->val : sym->alias->val;
        fprintf(fp, "#define %-30s %d\n", sym->name, val);
    }
    fprintf(fp, "\n#endif\n");
    fclose(fp);
    return 0;
}

static int write_binfile(const char *path)
{
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    if (fwrite(symtab_sbuf.buf, 1, symtab_sbuf.len, fp) != symtab_sbuf.len) {
        perror(path);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

static int cmp_hist_ent(const void *p1, const void *p2)
{
    uint32_t c1 = *(uint32_t *)p1 & 0xffffff;
    uint32_t c2 = *(uint32_t *)p2 & 0xffffff;

    return (c1 < c2) ? 1 :
           (c1 > c2) ? -1 :
           0;
}

typedef struct {
    sbuf_t sbuf;
    uint16_t shift;
    int nbits;
} enc64_t;

static void enc64_init(enc64_t *p, uint8_t *dict)
{
    sbuf_t dict_sbuf;

    memset(p, 0, sizeof(*p));
    sbuf_init(&p->sbuf);
    dict_sbuf.buf = dict;
    dict_sbuf.len = strlen((char *)dict);
    if (dict_sbuf.len != 59) {
        fprintf(stderr, "dict_sbuf.len: W:59/G:%d\n", dict_sbuf.len);
        exit(1);
    }
    sbuf_append(&p->sbuf, &dict_sbuf);
    if (p->sbuf.len != 59) {
        fprintf(stderr, "p->sbuf.len: W:59/G:%d\n", p->sbuf.len);
        exit(1);
    }
}

// pre          step              emit           post
// -------- 0 | ------aaaaaa  6 |    (nop)     | --aaaaaa 6
// --aaaaaa 6 | aaaaaabbbbbb 12 | >>4 aaaaaabb | ----bbbb 4
// ----bbbb 4 | --bbbbcccccc 10 | >>2 bbbbcccc | ------cc 2
// ------cc 2 | ----ccdddddd  8 | >>0 ccdddddd | -------- 0
//
static void enc64_emit(enc64_t *p, uint8_t b)
{
    p->shift = (p->shift << 6) | b;
    p->nbits += 6;
    if (p->nbits >= 8) {
        sbuf_push(&p->sbuf, p->shift >> (p->nbits - 8));
        p->nbits -= 8;
    }
}

// pre          emit
// -------- 0 |    (nop)
// --aaaaaa 6 | <<2 aaaaaa00
// ----bbbb 4 | <<4 bbbb0000
// ------cc 2 | <<6 cc000000
//
static void enc64_eos(enc64_t *p)
{
    enc64_emit(p, 0);
    if (p->nbits > 0) {
        sbuf_push(&p->sbuf, p->shift << (8 - p->nbits));
        p->nbits = 0;
    }
}

/*
 * 000000 (00)            eos
 * 000001 (01)            dict
 * 111011 (3b)            dict
 * 1111nn (3c..3f) nnnnnn literal
 */
static void encode_64(void)
{
    uint32_t hist[256];
    uint8_t dict[60];
    enc64_t enc64 = { 0 };
    uint8_t *p;
    sym_t *sym;
    const uint8_t *s;
    int i, c;

    // generate the character frequency table
    memset(hist, 0, sizeof(hist));
    for (i = 0; i < symtab_sbuf.len; i++) {
        ++hist[symtab_sbuf.buf[i]];
    }
    // byte value encoded in bits [31:24]
    for (i = 0; i < 256; i++) {
        hist[i] |= (i << 24);
    }
    // sort it (ignoring NULs)
    qsort(&hist[1], 255, sizeof(hist[0]), cmp_hist_ent);

    // make a dictionary out of the top 59 bytes
    for (i = 0; i < 59; i++) {
        dict[i] = hist[1 + i] >> 24;
    }
    dict[i] = '\0';

    // encode the string table, adjusting the offsets in the symtab.
    enc64_init(&enc64, dict);
    sym = symtab;
    sym->val = enc64.sbuf.len;
    s = symtab_sbuf.buf;
    for (i = 0; i < symtab_sbuf.len; i++) {
        c = *s++;
        if (c == '\0') {
            enc64_eos(&enc64);
            do {
                ++sym;
            } while (sym < &symtab[symtab_nsyms] && sym->alias != NULL);
            if (sym < &symtab[symtab_nsyms]) {
                sym->val = enc64.sbuf.len;
            }
        } else {
            p = (uint8_t *)strchr((char *)dict, c);
            if (p != NULL) {
                enc64_emit(&enc64, p - dict + 1);
            } else {
                enc64_emit(&enc64, 0x3c | c >> 6);
                enc64_emit(&enc64, c & 0x3f);
            }
        }
    }
    if (sym != &symtab[symtab_nsyms]) {
        fprintf(stderr, "Bogon: sym -> [%d], exp [%d]\n",
            (int)(sym - symtab), symtab_nsyms);
    }
    symtab_sbuf = enc64.sbuf;
}

int main(int argc, char *argv[])
{
    const char *progname = argv[0];
    const char *infile;
    const char *outh;
    const char *outbin;
    int verbose = 0;

    if (argc > 1 && strcmp(argv[1], "-v") == 0) {
        verbose = 1;
        ++argv;
        --argc;
    }

    if (argc != 4) {
        fprintf(stderr, "usage: %s [-v] infile out-h out-bin\n", progname);
        return 1;
    }
    infile = argv[1];
    outh = argv[2];
    outbin = argv[3];

    if (read_resfile(infile) < 0) {
        return 1;
    }
    if (verbose) {
        symtab_dump();
    }
    encode_64();
    if (verbose) {
        symtab_dump_enc64();
    }
    if (write_hfile(outh) < 0) {
        return 1;
    }
    if (write_binfile(outbin) < 0) {
        return 1;
    }

    return 0;
}

