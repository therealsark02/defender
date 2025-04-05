/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _ATTRACT_H_
#define _ATTRACT_H_

void early_irq_sleep(procfn_t fn, uint8_t ptime) __dead;
void early_irq_sucide(void) __dead;

static inline void attr_sleep(procfn_t fn, uint8_t ptime) __dead;
static void inline attr_sucide(void) __dead;

static inline void attr_sleep(procfn_t fn, uint8_t ptime)
{
    if (gd->evbi_finished) {
        sleep(fn, ptime);
    } else {
        early_irq_sleep(fn, ptime);
    }
}

static inline void attr_sucide(void)
{
    if (gd->evbi_finished) {
        sucide();
    } else {
        early_irq_sucide();
    }
}

void ramtest(void);
void amodes(void) __dead;
void defnnn(void);
void defend(void);
void logo(void) __dead;
void haldis(void) __dead;
void hallof(void);
void creds(void);
void defdraw(uint16_t x, uint8_t y);
void mkdefimg(void);
void scores(void);
void helpsw(void) __dead;
void ledret(void) __dead;
void plot_textimg(uint16_t x, uint8_t y, uint8_t nw, uint8_t h, const uint16_t *img);
struct hofent_s;
void haldt(const struct hofent_s *hof, uint16_t x, uint8_t y);

void attr_end_evbi(void);
void attr_wait_end(void);
void attr_resume(void);
void defend_resume(void);

#endif

