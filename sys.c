/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Program entry / sound effects loading / GEMDOS access.
 */

#include <stdarg.h>
#include "gd.h"
#include "vectors.h"
#include "screen.h"
#include "sysvars.h"
#include "timers.h"
#include "swtch.h"
#include "tos.h"
#include "irq.h"
#include "irq_thread.h"
#include "mem_limits.h"
#include "sys.h"

extern void _start(void);

gd_t g_gd;
#ifdef __MINT__
gd_t *gd = &g_gd;
#endif

static uint16_t os_pal[16];

uint8_t cfg_buf[512];
int cfg_len;
uint8_t hst_buf[512];
int hst_len;

#define FILE_ERR_STR_GEN(a, b) b,
const char *file_errs[FILE__NERRS] = {
    FILE_ERR_GENERATOR(FILE_ERR_STR_GEN)
};

static void u4out(uint8_t d)
{
    d &= 0xf;
    if (d <= 9) {
        gemdos_cconout('0' + d);
    } else {
        gemdos_cconout('A' + d - 10);
    }
}

static void u8out(uint8_t n)
{
    u4out(n >> 4);
    u4out(n);
}

static void u16out(uint16_t n)
{
    u8out(n >> 8);
    u8out(n);
}

static void u32out(uint32_t n)
{
    u16out(n >> 16);
    u16out(n);
}

static void strout(const char *s)
{
    if (s == NULL) {
        strout("<null>");
    } else {
        gemdos_cconws(s);
    }
}

static void decout(uint32_t n)
{
    char buf[32];
    char *s = buf;
    int r;

    do {
        r = n % 10;
        *s++ = r + '0';
        n /= 10;
    } while (n != 0);
    if (s == buf) {
        gemdos_cconout('0');
        return;
    }
    do {
        gemdos_cconout(*--s);
    } while (s != buf);
}

static void prf(const char *fmt, ...)
{
    va_list ap;
    char c;

    va_start(ap, fmt);
    while ((c = *fmt++) != '\0') {
        if (c == '%') {
            c = *fmt++;
            switch (c) {
            case '\0':
                --fmt;
                break;
            case '%':
                gemdos_cconout(c);
                break;
            case 'c':
                gemdos_cconout(va_arg(ap, int));
                break;
            case 'n':
                u4out(va_arg(ap, uint32_t));
                break;
            case 'b':
                u8out(va_arg(ap, unsigned int));
                break;
            case 'w':
                u16out(va_arg(ap, unsigned int));
                break;
            case 'l':
                u32out(va_arg(ap, uint32_t));
                break;
            case 's':
                strout(va_arg(ap, char *));
                break;
            case 'u':
                decout(va_arg(ap, unsigned int));
                break;
            default:
                gemdos_cconout('?');
                break;
            }
        } else {
            gemdos_cconout(c);
        }
    }
    va_end(ap);
}

struct cookie_s {
    uint32_t key;
    uint32_t value;
};

static int cookie_get(const char *kstr, uint32_t *valp)
{
    const struct cookie_s *p = (struct cookie_s *)_p_cookies;
    static uint32_t key;

    if (p == NULL) {
        return -1;
    }
    memcpy(&key, kstr, sizeof(key));
    while (p->key != 0) {
        if (p->key == key) {
            *valp = p->value;
            return 0;
        }
        ++p;
    }
    return -1;
}

static int get_features(void)
{
    static uint32_t val;
    int unsup = 0;

    gd->idata.mach = IDATA_MACH_ST;
    if (cookie_get("_MCH", &val) == 0) {
        switch (val) {
        case 0x00000:
            break;
        case 0x10000:
        case 0x10008:
            gd->idata.mach = IDATA_MACH_STE;
            gd->idata.features |= IDATA_FEAT_ENHJOY;
            break;
        case 0x30000:
            gd->idata.mach = IDATA_MACH_FALCON;
            gd->idata.features |= IDATA_FEAT_ENHJOY;
            break;
        case 0x10010:
            gd->idata.mach = IDATA_MACH_MEGA_STE;
            break;
        default:
            gd->idata.mach = IDATA_MACH_UNKNOWN;
            ++unsup;
            break;
        }
    }
    if (cookie_get("_SND", &val) == 0) {
        if (val & 0x2) {
            gd->idata.features |= IDATA_FEAT_SDMA;
        }
    }
    gd->idata.cpu = IDATA_CPU_68000;
    if (cookie_get("_CPU", &val) == 0) {
        switch (val) {
        case 0:
            break;
        case 30:
            gd->idata.cpu = IDATA_CPU_68030;
            break;
        default:
            gd->idata.cpu = IDATA_CPU_UNKNOWN;
            ++unsup;
            break;
        }
    }
    gd->idata.vid = IDATA_VID_ST;
    if (cookie_get("_VDO", &val) == 0) {
        switch (val >> 16) {
        case 0: // ST video
            break;
        case 1: // STE video
            gd->idata.vid = IDATA_VID_STE;
            break;
        case 3: // Falcon030 video
            gd->idata.vid = IDATA_VID_FALCON;
            break;
        default:
            gd->idata.vid = IDATA_VID_UNKNOWN;
            ++unsup;
            break;
        }
    }
    return unsup ? -1 : 0;
}

static const uint16_t pal[16] = {
    0x000, 0x007, 0x070, 0x077, 0x700, 0x707, 0x770, 0x770,
    0x000, 0x007, 0x070, 0x077, 0x700, 0x707, 0x770, 0x770,
};

static void save_os_screen(void)
{
    int i;

    gd->os_physbase = xbios_physbase();
    gd->os_logbase = xbios_logbase();
    if (gd->idata.vid == IDATA_VID_FALCON) {
        gd->os_mode = xbios_vsetmode(-1);
        gd->os_mon_type = xbios_mon_type();
    } else {
        gd->os_rez = xbios_getrez();
    }
    for (i = 0; i < 16; i++) {
        os_pal[i] = readw(0xffff8240 + 2 * i);
    }
    gd->os_screen_saved = 1;
}

static void restore_os_screen(void)
{
    if (gd->idata.vid == IDATA_VID_FALCON) {
        xbios_vsetscreen(gd->os_logbase, gd->os_physbase, 3, gd->os_mode);
    } else {
        xbios_setscreen(gd->os_logbase, gd->os_physbase, gd->os_rez);
    }
    xbios_wvbl();
    xbios_setpalette(os_pal);
}

static int is_mono(void)
{
    if (gd->idata.vid == IDATA_VID_FALCON) {
        return (gd->os_mon_type == 0);
    } else {
        return (gd->os_rez == 2);
    }
}

// setup the loading screen, like the
// bootloader would.
// Low res + palette.
// Base address not moved.
static void setup_loading_screen(void)
{
    uint16_t vmode;

    if (gd->idata.vid == IDATA_VID_FALCON) {
        vmode = 0xa2;
        if (gd->os_mon_type == 2) {
            vmode |= 0x110;
        }
        xbios_vsetscreen(-1, -1, 3, vmode);
    } else {
        xbios_setscreen(-1, -1, 0);
    }
    xbios_wvbl();
    xbios_setpalette(pal);
    xbios_wvbl();
}

// Setup the screen for the game.
// Move the screen down in memory,
// and for Falcon, adjust the height.
static void setup_game_screen(void)
{
    uint16_t vdb, vde;

    scrclr();
    xbios_setscreen(gd->vid_base, gd->vid_base, -1);
    xbios_wvbl();
    if (gd->idata.vid == IDATA_VID_FALCON) {
        vdb = readw(0xff82a8);
        vde = readw(0xff82aa);
        if (gd->os_mon_type == 2) {
            vdb -= 80;
            vde += 80;
            writew(0xff82a6, vdb);
            writew(0xff82a4, vde);
        } else {
            vdb -= 20;
            vde += 60;
        }
        writew(0xff82a8, vdb);
        writew(0xff82aa, vde);
    }
}

static void wait_space_and_pterm(void) __dead;
static void wait_space_and_pterm(void)
{
    while ((gemdos_cnecin() & 0xff) != ' ') {
        ; /* wait */
    }
    if (gd->os_screen_saved) {
        restore_os_screen();
    }
    gemdos_pterm(0);
}

static void center_msg(int row, const char *s)
{
    int width = gd->is_mono ? 80 : 40;
    int col = (width - strlen(s)) / 2;

    prf("\eY%c%c%s", 32+row, 32+col, s);
}

static void term_msg(const char *s) __dead;
static void term_msg(const char *s)
{
    if (gd->is_loading_screen) {
        xbios_setcolor(0, 0x300);
        xbios_setcolor(15, 0x542);
        xbios_wvbl();
    }
    prf("\eE");
    center_msg(12, s);
    if (!gd->idata.from_bootdisk) {
        center_msg(20, "press space to exit");
        wait_space_and_pterm();
    }
    for (;;) {
        xbios_wvbl();
    }
}

static void term_unsupported(void) __dead;
static void term_unsupported(void)
{
    if (gd->idata.mach == IDATA_MACH_UNKNOWN) {
        term_msg("Unsupported machine type.");
    } else if (gd->idata.cpu == IDATA_CPU_UNKNOWN) {
        term_msg("Unsupported CPU type.");
    } else if (gd->idata.vid == IDATA_VID_UNKNOWN) {
        term_msg("Unsupported video type.");
    } else {
        term_msg("Unsupported system.");
    }
}

static int os_error(int err)
{
    gd->gemdos_errno = err;
    switch (err) {
    case -33:   gd->file_err = FILE_ERR_NOFILE; break;
    case -34:   gd->file_err = FILE_ERR_NODIR; break;
    default:    gd->file_err = FILE_ERR_OSERR; break;
    }
    return -1;
}

static long load_file(const char *path, void *buf, long maxsz)
{
    short fd;
    long len;

    fd = gemdos_fopen(path, 0);
    if (fd < 0) {
        return os_error(fd);
    }
    len = gemdos_fread(fd, buf, maxsz);
    gemdos_fclose(fd);
    if (len < 0) {
        return os_error(len);
    }
    return len;
}

static int save_file(const char *path, const void *buf, long sz)
{
    short fd;
    long len;

    fd = gemdos_fcreate(path, 0);
    if (fd < 0) {
        return os_error(fd);
    }
    len = gemdos_fwrite(fd, buf, sz);
    gemdos_fclose(fd);
    if (len != sz) {
        gd->file_err = FILE_ERR_DISKFULL;
        return -1;
    }
    return 0;
}

static int load_sfxfile(int mem_dma_cap)
{
    const char *path;
    int32_t len;

    if ((gd->idata.features & IDATA_FEAT_SDMA) && mem_dma_cap) {
        path = "DMASFX.BIN";
    } else {
        path = "PSGSFX.BIN";
    }
    len = load_file(path, _end, gd->vid_base - (uint32_t)_end);
    if (len < 0) {
        return -1;
    }
    gd->idata.sfx = _end;
    gd->idata.sfxsize = len;

    return 0;
}

static const char cfg_path[] = "DEFENDER.CFG";

int load_cfg_file(void)
{
    int32_t len;

    memset(cfg_buf, 0, sizeof(cfg_buf));
    len = load_file(cfg_path, cfg_buf, sizeof(cfg_buf));
    if (len < 0) {
        cfg_len = 0;
        return len;
    }
    cfg_len = len;
    return 0;
}

int save_cfg_file(void)
{
    return save_file(cfg_path, cfg_buf, cfg_len);
}

static const char hst_path[] = "DEFENDER.HST";

int load_hst_file(void)
{
    int32_t len;

    memset(hst_buf, 0, sizeof(hst_buf));
    len = load_file(hst_path, hst_buf, sizeof(hst_buf));
    if (len < 0) {
        hst_len = 0;
        return -1;
    }
    hst_len = len;
    return 0;
}

int save_hst_file(void)
{
    return save_file(hst_path, hst_buf, hst_len);
}

#define OS_VECTOR_GENERATOR(mac) \
    mac(BUS_ERROR) \
    mac(ADDRESS_ERROR) \
    mac(ILLEGAL_INSTR) \
    mac(VBI) \
    mac(IKBD)

#define OS_VECTOR_ENUM_GEN(a) OS_V_##a,
enum {
    OS_VECTOR_GENERATOR(OS_VECTOR_ENUM_GEN)
    OS_V__NUM
};
#define OS_VECTOR_VECTOR_GEN(a) V_##a,
static uint32_t const os_vec_addrs[] = {
    OS_VECTOR_GENERATOR(OS_VECTOR_VECTOR_GEN)
};
static xcptfunc_t os_vector[OS_V__NUM];

static void save_os_vectors(void)
{
    int i;

    for (i = 0; i < OS_V__NUM; i++) {
        os_vector[i] = vector_get(os_vec_addrs[i]);
    }
}

static void restore_os_vectors(void)
{
    int i;

    for (i = 0; i < OS_V__NUM; i++) {
        vector_set(os_vec_addrs[i], os_vector[i]);
    }
}

static void save_os_mfp(void)
{
    gd->os_iera = readb(MFP_IERA);
    gd->os_ierb = readb(MFP_IERB);
    gd->os_imra = readb(MFP_IMRA);
    gd->os_imrb = readb(MFP_IMRB);
}

static void restore_os_mfp(void)
{
    timers_reset_all();
    timerc_setup(MFP_TMR_DIV_64, 0xc0);     // 200Hz
    timerd_setup(MFP_TMR_DIV_4, 0x02);      // 19200Hz x 16
    writeb(MFP_IERA, gd->os_iera);
    writeb(MFP_IERB, gd->os_ierb);
    writeb(MFP_IMRA, gd->os_imra);
    writeb(MFP_IMRB, gd->os_imrb);
}

void sys_set_error_vectors_game(void)
{
    vector_set(V_BUS_ERROR, (xcptfunc_t)xcpttab[0]);
    vector_set(V_ADDRESS_ERROR, (xcptfunc_t)xcpttab[1]);
    vector_set(V_ILLEGAL_INSTR, (xcptfunc_t)xcpttab[2]);
}

void sys_set_error_vectors_os(void)
{
    vector_set(V_BUS_ERROR, os_vector[OS_V_BUS_ERROR]);
    vector_set(V_ADDRESS_ERROR, os_vector[OS_V_ADDRESS_ERROR]);
    vector_set(V_ILLEGAL_INSTR, os_vector[OS_V_ILLEGAL_INSTR]);
}

void sys_enter(int from_bootdisk)
{
    uint32_t mem_capacity;

    gd->idata.from_bootdisk = from_bootdisk;

    if (get_features() < 0) {
        term_unsupported();
    }

    save_os_screen();
    save_os_vectors();
    save_os_mfp();

    gd->gemdos_version = gemdos_sversion();
    gd->tos_version = *(uint16_t *)((*(uint32_t *)0x4 & -(128 << 10)) + 2);

    if (!gd->idata.from_bootdisk) {
        prf("\ef");
        if (is_mono()) {
            gd->is_mono = 1;
            term_msg("DEFENDER requires a color monitor.");
        }
        prf("\eb%c", 15);
        setup_loading_screen();
        gd->is_loading_screen = 1;
        if (phystop < 0x100000) {
            term_msg("DEFENDER requires at least 1MB RAM.");
        }
        center_msg(12, "D E F E N D E R");
    } else {
        gd->is_loading_screen = 1;
    }

    // set the highest usable address
    gd->vid_base = (phystop - (160 * SCREEN_HEIGHT)) & -256;

    // can the game fit in the memory available?
    mem_capacity = gd->vid_base - (uint32_t)_start;
    if (mem_capacity < MEM_USED_PSG) {
        term_msg("Insufficient free RAM.");
    }

    // load the sfx file
    if (load_sfxfile(mem_capacity >= MEM_USED_DMA) < 0) {
        term_msg("File loading error.");
    }

    // load the config and scores
    load_cfg_file();
    load_hst_file();

    // transiton to the game screen
    setup_game_screen();

    // setup the heap
    heap = (void *)(((long)gd->idata.sfx + gd->idata.sfxsize + 3) & -4);

    // continue to the main code
    main();
}

static void sys_os_wrap_disable(void)
{
    sys_set_error_vectors_game();
    gd->os_enabled = 0;
}

/*
 * Turn the OS back on while using OS services (e.g. files)
 */
void sys_wrap_os(void (*func)(void *arg), void *arg)
{
    uint8_t thr_flag;

    thr_flag = irq_thread_disable();
    irq_disable();

    // call the OS VBI and Timer C hooks.
    gd->os_enabled = 1;
    gd->os_disable_timer = 0;

    // establish the OS vectors for redirected errors
    sys_set_error_vectors_os();


    // call os-accessing function
    irq_enable();
    func(arg);
    irq_disable();

    /*
     * Keep calling the OS VBI and Timer C dispatchers for
     * 5s to give floppy/hd time to quiesce.
     */
    gd->os_disable_timer = 250;

    // restore error vectors to game handlers
    sys_set_error_vectors_game();

    irq_enable();
    irq_thread_restore(thr_flag);
}

void sys_exit(void)
{
    irq_disable();
    restore_os_vectors();
    restore_os_mfp();
    keyboard_os_restore();
    irq_enable();
    restore_os_screen();
    gemdos_pterm(0);
}

void sys_vbi(void)
{
    if (gd->os_enabled) {
        // invoke OS vbl and Timer C hooks
        invoke_os_hooks(os_vector[OS_V_VBI]);
        if (gd->os_disable_timer) {
            if (--gd->os_disable_timer == 0) {
                sys_os_wrap_disable();
            }
        }
    }
}

