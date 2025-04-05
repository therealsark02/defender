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

void sbuf_init(sbuf_t *p)
{
    if (p->buf == NULL) {
        p->size = 4096;
        p->buf = malloc(p->size);
    }
    p->len = 0;
}

static void sbuf_makespace(sbuf_t *p, int req)
{
    if (p->len + req > p->size) {
        p->size *= 2;
        p->buf = realloc(p->buf, p->size);
    }
}

void sbuf_strcat(sbuf_t *p, const char *s)
{
    int len = strlen(s);
    int nlen;

    nlen = p->len + len + (p->len ? 0 : 1);
    sbuf_makespace(p, len + (p->len ? 0 : 1));
    memcpy(p->buf + (p->len ? p->len - 1 : 0), s, len + 1);
    p->len = nlen;
}

void sbuf_append(sbuf_t *p, const sbuf_t *q)
{
    sbuf_makespace(p, q->len);
    memcpy(p->buf + p->len, q->buf, q->len);
    p->len += q->len;
}

void sbuf_push(sbuf_t *p, uint8_t b)
{
    sbuf_makespace(p, 1);
    p->buf[p->len++] = b;
}

