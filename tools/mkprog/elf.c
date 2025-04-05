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
#include "dtls.h"

static elf_section_t *elf_section_byname(elf_t *elf, const char *name)
{
    int i;

    for (i = 1; i < elf->shnum; i++) {
        if (strcmp(elf->secttab[i].name, name) == 0) {
            return &elf->secttab[i];
        }
    }
    return NULL;
}

static void read_section(elf_t *elf, int i, elf_section_t *sect)
{
    Elf32_Shdr hdr;

    memcpy(&hdr, elf->buf + elf->shoff + i * elf->shentsize, sizeof(hdr));
    sect->shndx = i;
    sect->name = elf->shstrtab + bswap_32(hdr.sh_name);
    sect->type = bswap_32(hdr.sh_type);
    sect->flags = bswap_32(hdr.sh_flags);
    sect->addr = bswap_32(hdr.sh_addr);
    sect->offset = bswap_32(hdr.sh_offset);
    sect->size = bswap_32(hdr.sh_size);
    sect->link = bswap_32(hdr.sh_link);
    sect->info = bswap_32(hdr.sh_info);
    sect->addralign = bswap_32(hdr.sh_addralign);
    sect->entsize = bswap_32(hdr.sh_entsize);
    sect->elf = elf;
}

static int elf_sym_hash(const char *name)
{
    int hash = 0;
    char c;

    while ((c = *name++) != '\0') {
        hash = (hash << 4) ^ (hash >> 28) ^ c;
    }
    return hash & (SYMTAB_HASHSZ - 1);

}

static int read_symbol(elf_t *elf, uint32_t offs, elf_sym_t *sym)
{
    Elf32_Sym hdr;
    int h;

    memcpy(&hdr, elf->buf + offs, sizeof(hdr));
    sym->size = bswap_32(hdr.st_size);
    sym->other = hdr.st_other;
    sym->shndx = bswap_16(hdr.st_shndx);
    sym->type = ELF32_ST_TYPE(hdr.st_info);
    sym->bind = ELF32_ST_BIND(hdr.st_info);

    if (sym->type == STT_SECTION) {
        sym->name = elf->secttab[sym->shndx].name;
    } else {
        sym->name = elf->strtab + bswap_32(hdr.st_name);
    }
    if (sym->shndx == SHN_UNDEF) {
        fprintf(stderr, "%s undefined\n", sym->name);
        return -1;
    }
    sym->value = bswap_32(hdr.st_value);
    if (sym->shndx < elf->shnum) {
        sym->value += elf->secttab[sym->shndx].addr;
    }
    h = elf_sym_hash(sym->name);
    sym->hnext = elf->symtab_hash[h];
    elf->symtab_hash[h] = sym;

    return 0;
}

static void read_sections(elf_t *elf)
{
    int i;

    elf->secttab = calloc(elf->shnum, sizeof(elf_section_t));
    for (i = 0; i < elf->shnum; i++) {
        read_section(elf, i, &elf->secttab[i]);
    }
}

static int read_symbols(elf_t *elf)
{
    elf_section_t *sect = elf_section_byname(elf, ".symtab");
    int i, errs;

    if (sect == NULL || sect->type != SHT_SYMTAB) {
        fprintf(stderr, ".symtab not found\n");
        return -1;
    }
    elf->nsymtab = sect->size / sect->entsize;
    elf->symtab = calloc(sizeof(elf_sym_t), elf->nsymtab);
    errs = 0;
    for (i = 1; i < elf->nsymtab; i++) {
        if (read_symbol(elf, sect->offset + i * sect->entsize,
                &elf->symtab[i]) < 0) {
            ++errs;
        }
    }
    if (errs) {
        fprintf(stderr, "%d undefined symbols\n", errs);
        return -1;
    }

    return 0;
}

void dump_symbols(elf_t *elf)
{
    int text_shn = elf->text->shndx;
    int data_shn = elf->data->shndx;
    int bss_shn = elf->bss->shndx;
    elf_sym_t *sym;
    int i;

    sym = &elf->symtab[1];
    for (i = 1; i < elf->nsymtab; i++, sym++) {
        switch (sym->type) {
        case STT_NOTYPE:
        case STT_OBJECT:
        case STT_FUNC:
        case STT_SECTION:
        case STT_COMMON:
            printf("%08x ", sym->value);
            if (sym->shndx == text_shn) {
                putchar((sym->bind == STB_GLOBAL) ? 'T' : 't');
            } else if (sym->shndx == data_shn) {
                putchar((sym->bind == STB_GLOBAL) ? 'D' : 'd');
            } else if (sym->shndx == bss_shn) {
                putchar((sym->bind == STB_GLOBAL) ? 'B' : 'b');
            } else if (sym->shndx == SHN_ABS) {
                putchar('a');
            } else {
                putchar(' ');
            }
            printf(" %s\n", sym->name);
            break;
        default:
            break;
        }
    }
}

static int read_rela_sect(elf_t *elf, uint32_t base,
        elf_section_t *sect)
{
    int onrelatab = elf->nrelatab;
    Elf32_Rela hdr;
    elf_rela_t *rel;
    uint32_t offs, type, info;
    int i;

    elf->nrelatab += sect->size / sect->entsize;
    elf->relatab = realloc(elf->relatab,
            elf->nrelatab * sizeof(elf_rela_t));
    rel = &elf->relatab[onrelatab];
    offs = 0;
    for (i = onrelatab;
            i < elf->nrelatab; i++, rel++, offs += sect->entsize) {
        memcpy(&hdr, elf->buf + sect->offset + offs, sizeof(hdr));
        rel->offset = base + bswap_32(hdr.r_offset);
        info = bswap_32(hdr.r_info);
        type = ELF32_R_TYPE(info);
        rel->sym = &elf->symtab[ELF32_R_SYM(info)];
        rel->addend = bswap_32(hdr.r_addend);
        rel->value = rel->sym->value + rel->addend;
        if (type != R_68K_32) {
            fprintf(stderr, "rela type %d for %s not supported\n", type, rel->sym->name);
            return -1;
        }
    }
    return 0;
}

static int read_relas(elf_t *elf)
{
    static const char *names[] = { ".rela.text", ".rela.data" };
    elf_section_t *sect;
    uint32_t base;
    int i;

    for (i = 0; i < 2; i++) {
        sect = elf_section_byname(elf, names[i]);
        if (sect == NULL) {
            fprintf(stderr, "%s not found\n", names[i]);
            return -1;
        }
        if (sect->type != SHT_RELA) {
            fprintf(stderr, "%s not SHT_RELA\n", names[i]);
            return -1;
        }
        base = (i == 0) ? elf->text->addr : elf->data->addr;
        if (read_rela_sect(elf, base, sect) < 0) {
            return -1;
        }
    }
    return 0;
}

void dump_relas(elf_t *elf)
{
    elf_rela_t *rela;
    int i;

    rela = elf->relatab;
    for (i = 0; i < elf->nrelatab; i++, rela++) {
        printf("%08x %08x %s %x = %08x\n",
            rela->offset, rela->sym->value, rela->sym->name, rela->addend, rela->value);
    }
}

static int elf_init(elf_t *elf)
{
    elf_section_t *sect;
    Elf32_Ehdr ehdr;
    Elf32_Shdr shdr;

    if (elf->size < sizeof(ehdr)) {
        return -1;
    }
    memcpy(&ehdr, elf->buf, sizeof(ehdr));
    if (memcmp(ehdr.e_ident, ELFMAG, 4) != 0) {
        return -1;
    }
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32 ||
        ehdr.e_ident[EI_DATA] != ELFDATA2MSB ||
        ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
        return -1;
    }
    elf->type = bswap_16(ehdr.e_type);
    elf->machine = bswap_16(ehdr.e_machine);
    elf->version = bswap_32(ehdr.e_version);
    elf->entry = bswap_32(ehdr.e_entry);
    elf->phoff = bswap_32(ehdr.e_phoff);
    elf->shoff = bswap_32(ehdr.e_shoff);
    elf->flags = bswap_32(ehdr.e_flags);
    elf->ehsize = bswap_16(ehdr.e_ehsize);
    elf->phentsize = bswap_16(ehdr.e_phentsize);
    elf->phnum = bswap_16(ehdr.e_phnum);
    elf->shentsize = bswap_16(ehdr.e_shentsize);
    elf->shnum = bswap_16(ehdr.e_shnum);
    elf->shstrndx = bswap_16(ehdr.e_shstrndx);
    if (elf->type != ET_REL ||
        elf->machine != EM_68K ||
        elf->shentsize != sizeof(Elf32_Shdr)) {
        return -1;
    }
    memcpy(&shdr,
            elf->buf + elf->shoff + elf->shentsize * elf->shstrndx,
            sizeof(shdr));
    elf->shstrtab = (char *)elf->buf + bswap_32(shdr.sh_offset);
    read_sections(elf);
    elf->text = elf_section_byname(elf, ".text");
    if (elf->text == NULL) {
        fprintf(stderr, ".text not found\n");
        return -1;
    }
    if (elf->text->addr != 0) {
        fprintf(stderr, ".text not linked at address 0\n");
        return -1;
    }
    elf->data = elf_section_byname(elf, ".data");
    if (elf->data == NULL) {
        fprintf(stderr, ".data not found\n");
        return -1;
    }
    elf->bss = elf_section_byname(elf, ".bss");
    if (elf->bss == NULL) {
        fprintf(stderr, ".bss not found\n");
        return -1;
    }
    if (elf->data->addr != elf->text->addr + elf->text->size) {
        fprintf(stderr, ".data not contiguous to .text\n");
        return -1;
    }
    if (elf->data->offset != elf->text->offset + elf->text->size) {
        fprintf(stderr, ".data not file-contiguous to .text\n");
        return -1;
    }
    if (elf->bss->addr != elf->data->addr + elf->data->size) {
        fprintf(stderr, ".bss not contiguous to .data\n");
        return -1;
    }
    sect = elf_section_byname(elf, ".strtab");
    if (sect == NULL || sect->type != SHT_STRTAB) {
        fprintf(stderr, ".strtab not found\n");
        return -1;
    }
    elf->strtab = (char *)elf->buf + sect->offset;
    if (read_symbols(elf) < 0) {
        return -1;
    }
    //dump_symbols(elf);
    if (read_relas(elf) < 0) {
        return -1;
    }
    //dump_relas(elf);

    return 0;
}

int elf_load(elf_t *elf, const char *path)
{
    struct stat st;
    FILE *fp;

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    fstat(fileno(fp), &st);
    elf->size = st.st_size;
    elf->buf = (uint8_t *)malloc(elf->size);
    if (fread((char *)elf->buf, 1, elf->size, fp) != elf->size) {
        perror(path);
        goto err;
    }
    fclose(fp);
    return elf_init(elf);
err:
    fclose(fp);
    free((char *)elf->buf);
    return -1;
}

