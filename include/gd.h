/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

#ifndef _GD_H_
#define _GD_H_

#include "types.h"
#include "idata.h"
#include "lib.h"
#include "laser.h"
#include "proc.h"
#include "sound.h"
#include "panic.h"

struct pldata_s;
struct sprpat_s;
struct obj_s;
struct sfx_s;
struct enmyent_s;
struct textent_s;
struct note_s;
struct font_s;

typedef struct gd_s {
    // thread.S
    uint8_t irq_thread_state;       // must be offset 0
    uint8_t irq_nest;               // must be offset 1
    // raster.S
    uint8_t ras_mfp_imra;
    uint8_t ras_mfp_imrb;
    // sys
    idata_t idata;
    uint32_t os_physbase;
    uint32_t os_logbase;
    uint16_t os_rez;
    uint16_t os_mode;
    uint16_t os_mon_type;
    uint8_t os_screen_saved;
    uint8_t is_loading_screen;
    uint8_t is_mono;
    uint8_t os_iera;
    uint8_t os_ierb;
    uint8_t os_imra;
    uint8_t os_imrb;
    // proc.c
    proc_t *pfree;
    proc_t *spfree;
    proc_t *active;
    proc_t *crproc;
    // main.c
    uint32_t vid_base;
    // sys.c
    uint8_t file_err;
    int16_t gemdos_errno;
    uint16_t tos_version;
    uint16_t gemdos_version;
    uint8_t os_enabled;
    uint8_t os_disable_timer;
    // vbi_irq.c
    uint16_t frame;
    // irq_thread.c
    uint8_t curirq;
    uint8_t iflg;
    uint8_t pminy;                  // min y to plot
    uint8_t pmaxy;                  // max y to plot
    uint8_t want_timing;
    uint8_t want_grid;
    uint8_t grid_on;
    // obj.c
    struct obj_s *ofree;
    struct obj_s *iptr;
    struct obj_s *optr;
    struct obj_s *sptr;
    // exec.c
    uint8_t timer;
    uint8_t ovcnt;
    // player.c
    uint8_t status;         // game status
    uint8_t plaxc;          // screen x (u8) 2px prec
    uint16_t plascrx;       // screen x (u16) 1px prec
    uint8_t playc;          // screen y (u8)
    uint16_t nplascrx;      // new screen x (u16) 1px prec
    uint8_t nplaxc;         // new screen x (u8) 2px prec
    uint8_t nplayc;         // new screen y (u8)
    uint16_t plax16;        // screen x (u8.8)
    uint16_t play16;        // screen y (u8.8)
    uint16_t *opldst;       // old player location for erase
    const uint16_t *oplsrc; // old player location for erase
    int32_t plaxv;          // velocity x (s10.14)
    int32_t pladir;         // direction (s10.14)
    int32_t nplad;          // new direction (s10.14)
    int16_t playv;          // velocity y (s8.8)
    int16_t plabx;          // player global x (s10.6)
    int16_t bgl;            // background left (s10.6)
    int16_t bglx;           // old background left (s10.6)
    uint8_t pia01;
    uint8_t pia02;
    uint8_t pia21;
    uint8_t pia22;
    uint8_t pia31;
    uint8_t joy1;
    uint8_t revflg;
    // plstrt
    uint8_t credit;         // play credits (> 0 for play)
    uint8_t cunits;
    uint8_t bunits;
    uint8_t lccnt;
    uint8_t rccnt;
    uint8_t pwrflg;
    uint8_t curplr;         // current player (1 or 2)
    uint8_t pdflg;          // display prompt flag
    uint8_t plrcnt;         // player count
    uint8_t thflg;
    uint8_t lflg;
    uint8_t lasrpat;
    uint8_t sbflg;
    uint8_t scrflg;
    uint8_t bmbcnt;
    uint8_t astcnt;
    uint16_t repla;
    struct pldata_s *plrx;
    struct obj_s **tptr;
    // collision
    uint8_t pcflg;
    // prdisp
    uint8_t pridx;
    uint8_t fbidx;
    // swtch.c
    uint8_t kpi;            // producer index
    uint8_t kci;            // consumer index
    uint8_t kst;            // key parsing state
    uint8_t kargc;          // key arg count
    uint8_t kcmd;           // command
    uint8_t kidx;           // index for filling kargs
    uint8_t kargs[7];       // key args (op+args)
    uint8_t has_joypad;
    uint8_t killme;
    uint32_t jpad;
    uint32_t cheat_code;
    uint16_t cheats;
    // rand.c
    uint8_t seed;
    uint8_t hseed;
    uint8_t lseed;
    // stars.c
    uint8_t strcnt;
    // gexec.c
    uint8_t gtime;
    // enemies list
    uint8_t    lndres;      // ELIST
    uint8_t    tieres;
    uint8_t    prbres;
    uint8_t    sczres;
    uint8_t    swmres;
    uint8_t    wavtim;      // ELIST1
    uint8_t    wavsiz;
    uint8_t    lndxv;
    uint8_t    lndyvh;
    uint8_t    lndyvl;
    uint8_t    ldstim;
    uint8_t    tiexv;
    uint8_t    szry;
    uint8_t    szyvh;
    uint8_t    szyvl;
    uint8_t    szxv;
    uint8_t    szstim;
    uint8_t    swxv;
    uint8_t    swstim;
    uint8_t    swac;
    uint8_t    ufotim;
    uint8_t    ufstim;
    uint8_t    ufosk;
    uint8_t    lndcnt;      // ELEND, ECNTS
    uint8_t    tiecnt;
    uint8_t    prbcnt;
    uint8_t    sczcnt;
    uint8_t    swcnt;
    uint8_t    wavtmr;
    uint8_t    ufotmr;
    uint8_t    ufocnt;
                            // ECEND
    // appear.c
    void *lsexpl;
    uint8_t centmpx;
    uint8_t centmpy;
    // mess.c
    const struct font_s *font;
    uint16_t topl_x;
    uint8_t topl_y;
    uint16_t curser_x;
    uint8_t curser_y;
    uint8_t charsp;
    uint8_t linesp;
    // plend.c
    uint8_t pcent_x;
    uint8_t pcent_y;
    uint8_t pcolc;
    uint8_t psed;
    uint8_t lpsed;
    uint8_t psed2;
    uint8_t lpsed2;
    const uint8_t *pcolp;
    // early_irq.c
    proc_t **evbi_pplink;
    uint8_t emain_finished;
    uint8_t evbi_finished;
    uint8_t chars_ready;
    // attract.c
    uint8_t hsrflg;
    uint8_t startup;
    uint8_t lgoflg;
    uint8_t lgophase;
    uint8_t lgox;
    uint8_t lgoy;
    uint8_t lgoimg;
    int8_t  len;
    uint8_t pix;
    uint8_t dcolor;
    uint8_t defimg;
    uint8_t defani;
    uint8_t stalt;
    uint8_t lasreol;
    uint8_t *defwptr;
    const uint8_t *lgoptr;
    struct obj_s *shiptr;
    struct obj_s *manptr;
    struct obj_s *enemyp;
    struct obj_s *p500ob;
    const struct enmyent_s *otabpt;
    const struct textent_s *textmp;
    const struct textent_s *texptr;
    uint8_t manfrf;
    char inits[3];
    uint8_t initn;
    int8_t inidir;
    uint8_t uddel;
    uint8_t udcnt;
    uint8_t fircnt;
    uint8_t firflg;
    uint8_t pnumb;
    uint8_t entflg;
    uint8_t alltime_dirty;
    uint32_t pscore;
    // panic.c
    uint8_t panic_row;
    uint8_t panic_col;
    // sound.c
    uint8_t sndtmr;
    uint8_t sndpri;
    uint8_t sndrep;
    const uint8_t *sndx;
    uint8_t snddis;
    struct sfx_s *cursnd;
    uint8_t issnddma;
    // tunes.c
    const struct note_s *tune_ptr;
    const struct note_s *tune_end;
    uint8_t tune_playing;
    uint8_t tune_cntdn;
    // tie.c
    uint8_t tflg;
    const struct sprpat_s *bax;
} gd_t;
#define gd_lndyv()  ((gd->lndyvh << 8) | gd->lndyvl)
#define gd_szyv()  ((gd->szyvh << 8) | gd->szyvl)

extern gd_t g_gd;
#ifdef __MINT__
extern gd_t *gd;
#else
register gd_t *gd asm ("a6");
#endif

// status
#define ST_GAMEOV           0x80
#define ST_ATTRACT          0x40
#define ST_NOSTOBJS         0x20
#define ST_PLNOCOLL         0x10
#define ST_PLDEAD           0x08
#define ST_APPEAREXPLDIS    0x04
#define ST_NOTERRAIN        0x02
#define ST_PLSTARTDELAY     0x01

// cheats
#define CHEAT_INVINCIBLE    0x01
#define CHEAT_SCORE         0x02
#define CHEAT_SBMAGIC       0x04
#define CHEAT_DEVMENUS      0x08

// pia
#define PIA01_AUTOUP        0x01
#define PIA01_ADVANCE       0x02
#define PIA01_RCOIN         0x04
#define PIA01_HSRESET       0x08
#define PIA01_LCOIN         0x10
#define PIA01_CCOIN         0x20

#define PIA21_DOWN          0x80
#define PIA21_REVERSE       0x40
#define PIA21_1PLAYER       0x20
#define PIA21_2PLAYERS      0x10
#define PIA21_HYPERSPACE    0x08
#define PIA21_SMARTBOMB     0x04
#define PIA21_THRUST        0x02
#define PIA21_FIRE          0x01

#define PIA31_UP            0x01

#endif

