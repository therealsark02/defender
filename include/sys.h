/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _SYS_H_
#define _SYS_H_

extern uint8_t cfg_buf[512];
extern int cfg_len;
extern uint8_t hst_buf[512];
extern int hst_len;

void sys_vbi(void);

void sys_set_error_vectors_game(void);
void invoke_os_hooks(void *os_vbi_func);
void sys_wrap_os(void (*func)(void *arg), void *arg);
void sys_exit(void);
int load_cfg_file(void);
int save_cfg_file(void);
int load_hst_file(void);
int save_hst_file(void);

#define FILE_ERR_GENERATOR(mac) \
    mac(OK,         NULL) \
    mac(DISKFULL,   "Disk full") \
    mac(NOFILE,     "File not found") \
    mac(NODIR,      "Directory not found") \
    mac(INVALID,    "File invalid") \
    mac(OSERR,      "OS Error")

#define FILE_ERR_ENUM_GEN(a, b) FILE_ERR_##a,
enum {
    FILE_ERR_GENERATOR(FILE_ERR_ENUM_GEN)
    FILE__NERRS
};

extern const char *file_errs[FILE__NERRS];

#endif

