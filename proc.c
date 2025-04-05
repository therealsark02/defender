/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Process handling. A process represents an independent game behavior
 * that persists over multiple frames. It contains a function pointer and
 * a timer (in frames, to schedule the calling of the function), as well as
 * space to store process-specifc data.
 * Two types of process are available: a normal sized process and a
 * super-process, named for its larger data area. Only ties (bombers) use
 * super-processes, to hold all the objects in their squad. 
 * Process functions are run-to-completion, and must end with either a
 * sleep() - rescheduling another call, or a sucide() - killing the process.
 */

#include "gd.h"
#include "irq.h"
#include "exec.h"

proc_t ptab[NPROCS];
sproc_t sptab[NSPROCS];

void pinit(void)
{
    int i;

    gd->pfree = &ptab[0];
    for (i = 0; i < NPROCS - 1; i++) {
        ptab[i].plink = &ptab[i + 1];
    }
    ptab[i].plink = NULL;
    gd->active = NULL;

    gd->spfree = (proc_t *)&sptab[0];
    for (i = 0; i < NSPROCS - 1; i++) {
        sptab[i].plink = (proc_t *)&sptab[i + 1];
    }
    sptab[i].plink = NULL;

    gd->crproc = (proc_t *)&gd->active;
}

proc_t *mkproc(procfn_t fn, uint8_t ptype)
{
    proc_t *p;

    p = gd->pfree;
    if (p == NULL) {
        error();
    }
    gd->pfree = p->plink;
    p->pcod = 0;
    p->paddr = fn;
    p->ptype = ptype;
    p->ptime = 1;
    p->plink = gd->crproc->plink;
    gd->crproc->plink = p;
    return p;
}

sproc_t *msproc(procfn_t fn, uint8_t ptype)
{
    proc_t *p;

    p = gd->spfree;
    if (p == NULL) {
        error();
    }
    gd->spfree = p->plink;
    p->pcod = 1;            // sproc_t
    p->paddr = fn;
    p->ptype = ptype;
    p->ptime = 1;
    p->plink = gd->crproc->plink;
    gd->crproc->plink = p;
    return (sproc_t *)p;
}

void gncide(void)
{
    proc_t *p;

    for (p = gd->active; p != NULL; p = p->plink) {
        if (p != gd->crproc && p->ptype != CTYPE) {
            p = kill(p);
        }
    }
}

proc_t *kill(proc_t *p)
{
    proc_t *pp;

    for (pp = (proc_t *)&gd->active; pp != NULL; pp = pp->plink) {
        if (pp->plink == p) {
            pp->plink = p->plink;
            if (p->pcod == 0) {
                p->plink = gd->pfree;
                gd->pfree = p;
            } else {
                p->plink = gd->spfree;
                gd->spfree = p;
            }
            return pp;
        }
    }
    error();
}

