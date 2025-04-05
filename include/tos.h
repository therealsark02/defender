/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _TOS_H_
#define _TOS_H_

typedef struct {
    uint8_t  d_reserved[21];
    uint8_t  d_attrib;
    uint16_t d_time;
    uint16_t d_date;
    uint32_t d_length;
    char     d_fname[14];
} DTA;

uint16_t gemdos_sversion(void);
int16_t gemdos_fcreate(const char *path, uint16_t mode);
int16_t gemdos_fopen(const char *path, uint16_t mode);
int16_t gemdos_fclose(uint16_t fd);
long gemdos_fread(uint16_t fd, void *buf, uint32_t len);
long gemdos_fwrite(uint16_t fd, const void *buf, uint32_t len);
long gemdos_fseek(uint16_t fd, uint32_t offset, uint16_t whence);
void gemdos_super(void *sp);
void gemdos_cconout(int c);
void gemdos_cconws(const char *s);
uint16_t gemdos_cnecin(void);
void gemdos_pterm(uint16_t res) __dead;

uint16_t gemdos_dgetdrv(void);
int32_t gemdos_dsetdrv(uint16_t drv);
void gemdos_dgetpath(char *buf, uint16_t driveno);
int16_t gemdos_dsetpath(const char *path);
DTA *gemdos_fgetdta(void);
int16_t gemdos_fsfirst(const char *fspec, uint16_t attrs);
int16_t gemdos_fsnext(void);

long xbios_physbase(void);
long xbios_logbase(void);
uint16_t xbios_getrez(void);
uint16_t xbios_mon_type(void);
void xbios_vsetscreen(long logaddr, long physaddr, short rez, short mode);
void xbios_setscreen(long logaddr, long physaddr, short res);
int16_t xbios_setcolor(short colornum, short color);
uint16_t xbios_vsetmode(uint16_t mode);
void xbios_setpalette(const uint16_t *pal);
void xbios_wvbl(void);
#endif

