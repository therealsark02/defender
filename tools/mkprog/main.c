/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "dtls.h"

static const char *progname;

static uint8_t *make_outbuf(elf_t *elf, uint32_t *lenp)
{
    uint32_t relocs_offs, text_offs, doffset;
    uint32_t data_offs, outlen, prev_offset;
    int shndx, i, nrela, first;
    const elf_rela_t *rel;
    uint8_t *outbuf;
    uint8_t *rwp;
    phdr_t phdr;

    // count non-abs relocs
    rel = elf->relatab;
    nrela = 0;
    for (i = 0; i < elf->nrelatab; i++, rel++) {
        shndx = rel->sym->shndx;
        if (shndx >= elf->shnum) {
            if (shndx != SHN_ABS) {
                fprintf(stderr, "unexpected rela %s, shndx %d\n", rel->sym->name, shndx);
                return NULL;
            }
        } else {
            ++nrela;
        }
    }

    // phdr
    // text
    // data
    // relocs
    text_offs = sizeof(phdr_t);
    data_offs = text_offs + elf->text->size;
    relocs_offs = data_offs + elf->data->size;
    outlen = relocs_offs + 4 * nrela;
    outbuf = calloc(1, outlen);

    // build phdr
    phdr.ph_branch = bswap_16(0x601a);
    phdr.ph_tlen = bswap_32(elf->text->size);
    phdr.ph_dlen = bswap_32(elf->data->size);
    phdr.ph_blen = bswap_32(elf->bss->size);
    phdr.ph_slen = 0;
    phdr.ph_res1 = 0;
    phdr.ph_prgflags = 0;
    phdr.ph_absflag = 0;
    memcpy(outbuf, &phdr, sizeof(phdr));

    // copy text and data
    memcpy(outbuf + text_offs, elf->buf + elf->text->offset,
        elf->text->size + elf->data->size);

    // build relocs and patch text/data
    rwp = outbuf + relocs_offs;
    rel = elf->relatab;
    first = 1;
    for (i = 0; i < elf->nrelatab; i++, rel++) {
        if (rel->offset >= elf->text->size + elf->data->size) {
            fprintf(stderr, "rela offset overrun\n");
            return NULL;
        }
        if (*(uint32_t *)(outbuf + text_offs + rel->offset) != 0) {
            fprintf(stderr, "Pre-reloc contents bogon\n");
            return NULL;
        }
        // patch image
        *(uint32_t *)(outbuf + text_offs + rel->offset) = bswap_32(rel->value);
        if (rel->sym->shndx != SHN_ABS) {
            // add to runtime relocs
            if (first) {
                *(uint32_t *)rwp = bswap_32(rel->offset);
                rwp += 4;
                first = 0;
            } else {
                if (rel->offset < prev_offset) {
                    fprintf(stderr, "backwards bogon\n");
                    return NULL;
                }
                doffset = rel->offset - prev_offset;
                while (doffset > 0xfe) {
                    *rwp++ = 0x1;
                    doffset -= 0xfe;
                }
                if (doffset > 0) {
                    *rwp++ = doffset;
                }
            }
            prev_offset = rel->offset;
        }
    }
    *rwp++ = 0;
    if (rwp > outbuf + outlen) {
        fprintf(stderr, "outbuf overrun\n");
        exit(1);
    }
    outlen = rwp - outbuf;
    *lenp = outlen;
    return outbuf;
}

static int write_outbuf(const uint8_t *buf, uint32_t len,
        const char *path)
{
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    if (fwrite(buf, 1, len, fp) != len) {
        perror(path);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

static void exit_usage(void) __attribute__((noreturn));
static void exit_usage(void)
{
    fprintf(stderr, "usage: %s -o outfile infile\n", progname);
    exit(1);
}

int main(int argc, char *argv[])
{
    elf_t *elf = calloc(1, sizeof(*elf));
    const char *outpath = NULL;
    const char *inpath;
    uint32_t outlen;
    uint8_t *outbuf;
    int c;

    progname = argv[0];
    while ((c = getopt(argc, argv, "o:")) != -1) {
        switch (c) {
        case 'o':
            outpath = optarg;
            break;
        default:
            exit_usage();
        }
    }
    argc -= optind;
    argv += optind;
    if (argc < 1) {
        exit_usage();
    }
    inpath = argv[0];

    if (elf_load(elf, inpath) < 0) {
        return 1;
    }

    outbuf = make_outbuf(elf, &outlen);
    if (outbuf == NULL) {
        return 1;
    }
    if (write_outbuf(outbuf, outlen, outpath) < 0) {
        return 1;
    }
    return 0;
}

