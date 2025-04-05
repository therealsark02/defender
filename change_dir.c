/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Change Directories: Navigate through the GEMDOS hierarchy to
 * allow a new directory to be selected for load/save.
 */

#include "gd.h"
#include "screen.h"
#include "tos.h"
#include "sprites.h"
#include "tdisp.h"
#include "mess.h"
#include "setup.h"
#include "sys.h"

static char pathbuf[256];
static char dirent[32][14];
static int ndirent;

static void show_path(void)
{
    uint16_t drv = gemdos_dgetdrv();
    int len = strlen(pathbuf);

    if (len > 33) {
        messf(16, 58,
            ID_SHOW_PATH_LONG,
            'A' + drv, pathbuf + len - 31);
    } else {
        messf(160 - (len + 2) * 4, 58,
            ID_SHOW_PATH_SHORT,
            'A' + drv, pathbuf,
            (pathbuf[0] == '\0') ? "\\" : "");
    }
}

static void pathbuf_append(const char *dir)
{
    int len1 = strlen(pathbuf);
    int len2 = strlen(dir);

    if (len1 + 1 + len2 + 1 > sizeof(pathbuf)) {
        return;
    }
    pathbuf[len1] = '\\';
    memcpy(pathbuf + len1 + 1, dir, len2 + 1);
}

static char *strrchr(const char *s, int search)
{
    char *res = NULL;
    int c;
    
    while ((c = *s++) != '\0') {
        if (c == search) {
            res = (char *)(s - 1);
        }
    }
    return res;
}

static void fill_dirent(void)
{
    DTA *dta = gemdos_fgetdta();
    int32_t r;

    r = gemdos_fsfirst("*.*", 0x10);
    ndirent = 0;
    memcpy(dirent[ndirent++], "..", 3);
    while (r == 0 && ndirent < 32) {
        if (dta->d_attrib & 0x10) {
            memcpy(dirent[ndirent++], dta->d_fname, sizeof(dirent[0]));
        }
        r = gemdos_fsnext();
    }
}

static int select_dirent(void)
{
    int key, row, win_start, k;

    win_start = 0;
    do {
        gd->curser_y = 78;
        blkclr(96, gd->curser_y, 16 * 8, 10 * gd->linesp);
        key = 1;
        row = win_start;
        while (key < 8 && row < ndirent) {
            messf(96, gd->curser_y,
                ID_SHOW_DIRENT_ENTRY,
                key++, dirent[row++]);
        }
        gd->curser_y += gd->linesp * (1 + 8 - key);
        if (win_start != 0) {
            messf(96, gd->curser_y, ID_SHOW_DIRENT_UP);
        } else {
            gd->curser_y += gd->linesp;
        }
        if (row < ndirent) {
            messf(96, gd->curser_y, ID_SHOW_DIRENT_DOWN);
        } else {
            gd->curser_y += gd->linesp;
        }
        gd->curser_y += gd->linesp;
        messf(96, gd->curser_y, ID_BACK);

        k = setup_getnumkey(9);
        if (k == 8 && win_start > 0) {
            win_start -= 7;
        } else if (k == 9 && win_start + 7 < ndirent) {
            win_start += 7;
        }
    } while (k > 0 && (k > 7 || (win_start + k) > ndirent));

    return (k == 0) ? -1 : win_start + k - 1;
}

static void choose_drive(void)
{
    uint16_t odrv = gemdos_dgetdrv();
    int drv_valid = 0;
    uint16_t drvmap;
    int i, row, r;

    scrclr();
    setup_title(ID_CHANGE_DIRECTORY);

    drvmap = gemdos_dsetdrv(odrv);
    ndirent = 0;
    for (i = 0; i < 16; i++) {
        if (drvmap & (1 << i)) {
            dirent[ndirent][0] = 'A' + i;
            dirent[ndirent][1] = ':';
            dirent[ndirent][2] = '\0';
            ++ndirent;
        }
    }
    do {
        row = select_dirent();
        if (row >= 0) {
            gemdos_dsetdrv(dirent[row][0] - 'A');
            r = gemdos_fsfirst("*.*", 0);
            if (r < 0 && r != -33) {
                setup_report(ID_DRIVE_NOT_VALID);
                blkclr(160 - (8 * 8), 220, 16 * 8, 8);
                drv_valid = 0;
            } else {
                drv_valid = 1;
            }
        }
    } while (row >= 0 && r < 0 && r != -33);
    if (!drv_valid) {
        gemdos_dsetdrv(odrv);
    }
}

void do_change_dir(void *arg)
{
    char *p;
    int row;

    do {
        gemdos_dgetpath(pathbuf, 0);
        fill_dirent();

        scrclr();
        setup_title(ID_CHANGE_DIRECTORY);

        show_path();

        row = select_dirent();
        if (row >= 0) {
            if (row == 0) {
                p = strrchr(pathbuf, '\\');
                if (p != NULL) {
                    if (p == pathbuf) {
                        p[1] = '\0';
                    } else {
                        *p = '\0';
                    }
                    gemdos_dsetpath(pathbuf);
                } else {
                    choose_drive();
                }
            } else {
                pathbuf_append(dirent[row]);
                gemdos_dsetpath(pathbuf);
            }
        }
    } while (row >= 0);
}

void change_dir(uint8_t id)
{
    sys_wrap_os(do_change_dir, NULL);
}

