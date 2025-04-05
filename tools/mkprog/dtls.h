/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef __DTLS_H__
#define __DTLS_H__

#include <byteswap.h>
#include <elf.h>

struct elf_s;

#define SYMTAB_HASHSZ   512
typedef struct elf_sym_s {
    struct elf_sym_s *hnext;
    const char *name;
    uint32_t value;
    uint32_t size;
    int type;
    int bind;
    int other;
    int shndx;
} elf_sym_t;

typedef struct elf_section_s {
    struct elf_s *elf;
    const char *name;
    int shndx;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
} elf_section_t;

typedef struct {
    uint32_t offset;
    uint32_t value;
    elf_sym_t *sym;
    int32_t addend;
} elf_rela_t;

typedef struct elf_s {
    const uint8_t *buf;
    uint32_t size;
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
    const char *shstrtab;
    const char *strtab;
    elf_section_t *secttab;
    elf_section_t *text;
    elf_section_t *data;
    elf_section_t *bss;
    elf_sym_t *symtab;
    int nsymtab;
    elf_sym_t *symtab_hash[SYMTAB_HASHSZ];
    elf_rela_t *relatab;
    int nrelatab;
} elf_t;
int elf_load(elf_t *elf, const char *path);

typedef struct {
    uint16_t  ph_branch;
    uint32_t  ph_tlen;
    uint32_t  ph_dlen;
    uint32_t  ph_blen;
    uint32_t  ph_slen;
    uint32_t  ph_res1;
    uint32_t  ph_prgflags;
    uint16_t  ph_absflag;
} __attribute__((packed)) phdr_t;

#endif

