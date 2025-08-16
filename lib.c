/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Miscellaneous 'C' and GEMDOS functions.
 */

#include "gd.h"

#pragma GCC optimize("Os")

void *heap;

typedef struct {
    void *ptr;
    void *end;
} malloc_region_t;

#define N_MALLOC_REGIONS    2
malloc_region_t m_region[N_MALLOC_REGIONS];
int m_region_idx;

void heap_add(void *m, uint32_t size)
{
    malloc_region_t *r;
    void *eptr = m + size;
    void *sptr = m;
    uint16_t i;

    // ensure start/end are aligned
    if ((uint32_t)sptr & 1) {
        ++sptr;
    }
    if ((uint32_t)eptr & 1) {
        --eptr;
    }
    if (m_region_idx < N_MALLOC_REGIONS) {
        r = &m_region[m_region_idx];
        for (i = 0; i < m_region_idx; i++) {
            if (m < m_region[i].ptr) {
                break;
            }
        }
        if (i < m_region_idx) {
            *r = m_region[i];
            r = &m_region[i];
        }
        r->ptr = sptr;
        r->end = eptr;
        ++m_region_idx;
    } else {
        panic(0xdeb6f388);
    }
}

void *malloc(uint32_t size)
{
    uint16_t i;
    void *p;

    size = (size + 1) & -2;

    // first try reclaimed regions
    for (i = 0; i < m_region_idx; i++) {
        if (m_region[i].end - m_region[i].ptr >= size) {
            p = m_region[i].ptr;
            m_region[i].ptr += size;
            return p;
        }
    }

    // use the main heap
    if ((char *)heap + size > (char *)gd->vid_base) {
        panic(0xdeadeee, heap, size,
                (char *)heap + size, gd->vid_base);
    }
    p = heap;
    heap = (char *)heap + size;
    return p;
}

void memzero(void *dst, uint32_t sz)
{
    uint8_t *cdst = dst;;
    uint32_t *wdst;

    while (sz && ((uint32_t)cdst & 0x3)) {
        *cdst++ = 0;
        --sz;
    }
    wdst = (uint32_t *)cdst;
    while (sz >= 4) {
        *wdst++ = 0;
        sz -= 4;
    }
    cdst = (uint8_t *)wdst;;
    while (sz--) {
        *cdst++ = 0;
    }
}

void *zalloc(uint32_t size)
{
    void *p = malloc(size);
    memzero(p, size);
    return p;
}

void *memset(void *dst, int c, uint32_t size)
{
    int nlongs = size >> 2;
    uint8_t *bdst, *e_bdst;
    uint32_t *ldst = dst;
    uint32_t *e_ldst;
    uint32_t ldata;

    if (((uint32_t)dst & 0x1) == 0) {
        // dst aligned
        ldata = ((uint8_t)c << 24) |
                ((uint8_t)c << 16) |
                ((uint8_t)c << 8) |
                (uint8_t)c;
        e_ldst = ldst + nlongs;
        while (ldst != e_ldst) {
            *ldst++ = ldata;
        }
        size -= nlongs << 2;
    }
    bdst = (uint8_t *)ldst;
    e_bdst = bdst + size;
    while (bdst != e_bdst) {
        *bdst++ = (uint8_t)c;
    }
    return dst;
}

void *memcpy(void *dst, const void *src, uint32_t size)
{
    const uint32_t *lsrc = src;
    int nlongs = size >> 2;
    uint8_t *bdst, *e_bdst;
    uint32_t *ldst = dst;
    const uint8_t *bsrc;
    uint32_t *e_ldst;

    if ((((uint32_t)dst | (uint32_t)src) & 0x1) == 0) {
        // src & dst aligned
        e_ldst = ldst + nlongs;
        while (ldst != e_ldst) {
            *ldst++ = *lsrc++;
        }
        size -= nlongs << 2;
    }
    bsrc = (uint8_t *)lsrc;
    bdst = (uint8_t *)ldst;
    e_bdst = bdst + size;
    while (bdst != e_bdst) {
        *bdst++ = *bsrc++;
    }
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s1 == *s2) {
        ++s1; ++s2;
    }
    return *s1 - *s2;
}

int strlen(const char *s)
{
    const char *p;

    for (p = s; *p; p++) {
    } 
    return (int)(p - s);
}

void error(void)
{
    for (;;);
}
