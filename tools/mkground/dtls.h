/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _DTLS_H_
#define _DTLS_H_

// data.c
extern const uint8_t tdata[];
extern const size_t tdatasz;
extern const uint8_t mterr[];
extern const size_t mterrsz;

// ground.c
typedef struct {
    int w;          // width in pixels
    int h;          // height in pixels
    uint8_t *data;
} bitmap_t;
bitmap_t *create_ground_bitmap(void);
bitmap_t *create_mterr_bitmap(void);

#endif
