/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#include <stdio.h>
#include <stdint.h>
#include "dtls.h"

static int save_bitmap(const bitmap_t *bm, const char *path)
{
    size_t sz;
    FILE *fp;

    fp = fopen(path, "w");
    if (fp == NULL) {
        perror(path);
        return -1;
    }
    sz = bm->w * bm->h / 8;
    if (fwrite(bm->data, 1, sz, fp) != sz) {
        perror(path);
        return -1;
    }
    if (fclose(fp) == EOF) {
        perror(path);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    bitmap_t *gbm;
    bitmap_t *gtbm;

    if (argc < 3) {
        fprintf(stderr, "usage: %s pathground pathmterr\n", argv[0]);
        return 1;
    }

    gbm = create_ground_bitmap();
    gtbm = create_mterr_bitmap();

    if (save_bitmap(gbm, argv[1]) < 0) {
        return 1;
    }
    if (save_bitmap(gtbm, argv[2]) < 0) {
        return 1;
    }
    return 0;
}
