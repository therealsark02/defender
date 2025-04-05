/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _LASER_H_
#define _LASER_H_

typedef struct {
    uint16_t *dst;
    int8_t shift;
} laser_t;

void lfire(void);
void laser_init(void);
void attr_lasr(void);

#endif

