/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _LIB_H_
#define _LIB_H_

#define __dead __attribute__((noreturn))

// linker
extern char _end[];

// lib.c
extern void *heap;
void heap_add(void *m, uint32_t size);

// main.c
int main(void);

// lib.c
void *malloc(uint32_t size);
void *zalloc(uint32_t size);
void memzero(void *dst, uint32_t size);
void *memset(void *dst, int c, uint32_t size);
void *memcpy(void *dst, const void *src, uint32_t size);
int strcmp(const char *s1, const char *s2);
int strlen(const char *s);

// register i/o
static inline uint8_t readb(uint32_t addr)
{
    uint8_t res = *(volatile uint8_t *)addr;
    asm volatile("" ::: "memory");
    return res;
}

static inline void writeb(uint32_t addr, uint8_t data)
{
    *(volatile uint8_t *)addr = data;
    asm volatile("" ::: "memory");
}

static inline uint16_t readw(uint32_t addr)
{
    uint16_t res = *(volatile uint16_t *)addr;
    asm volatile("" ::: "memory");
    return res;
}

static inline void writew(uint32_t addr, uint16_t data)
{
    *(volatile uint16_t *)addr = data;
    asm volatile("" ::: "memory");
}

static inline void writel(uint32_t addr, uint32_t data)
{
    *(volatile uint32_t *)addr = data;
    asm volatile("" ::: "memory");
}

#endif

