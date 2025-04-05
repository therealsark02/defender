/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Setup / configuration / information menus.
 */

#include "gd.h"
#include "screen.h"
#include "data.h"
#include "sprites.h"
#include "color.h"
#include "mess.h"
#include "sysvars.h"
#include "change_dir.h"
#include "irq_thread.h"
#include "keys.h"
#include "tdisp.h"
#include "expl.h"
#include "tos.h"
#include "menu.h"
#include "font.h"
#include "attract.h"
#include "mem_limits.h"
#include "sys.h"
#include "swtch.h"
#include "cmos.h"
#include "setup.h"

extern const char version_str[];
extern const char git_rev[];
extern const char build_date[];

static uint8_t curkey;

static void waitvbi(void)
{
    uint16_t frame = gd->frame;

    while (gd->frame == frame) {
        asm volatile("" ::: "memory");
    }
}

static void delay(uint16_t cnt)
{
    while (cnt--) {
        waitvbi();
    }
}

static void keyhandler(uint16_t event)
{
    if (EV_TYPE(event) != EV_KEYBOARD || !EV_KEYDN(event)) {
        return;
    }
    curkey = EV_KEY(event);

    // cheat codes
    if (curkey == KEY_ESC) {
        gd->cheat_code = 0;
    } else {
        gd->cheat_code = (gd->cheat_code << 5) ^ (gd->cheat_code >> 27) ^ curkey;
        if (gd->cheat_code == 0xe7986934) { // goironman
            gd->cheats |= CHEAT_DEVMENUS;
        }
    }
}

static uint8_t getkey(void)
{
    curkey = 0xff;
    do {
        waitvbi();
        keyscan(keyhandler);
    } while (curkey == 0xff);
    return curkey;
}

uint8_t setup_getnumkey(uint8_t top)
{
    static const uint8_t kmap[10] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 0
    };
    static const uint8_t kpmap[10] = {
        7, 8, 9, 4, 5, 6, 1, 2, 3, 0
    };
    uint8_t k, n;

    do {
        do {
            k = getkey();
        } while (!((k >= KEY_1 && k <= KEY_0) || 
                  (k >= KEY_KP7 && k <= KEY_KP0)));
        if (k >= KEY_1 && k <= KEY_0) {
            n = kmap[k - KEY_1];
        } else {
            n = kpmap[k - KEY_KP7];
        }
    } while (n != 0 && n > top);
    return n;
}

static uint16_t swidth(const char *s)
{
    uint16_t w = 0;
    char c;

    while ((c = *s++) != '\0') {
        if (c >= ' ') {
            w += (c == ' ') ? 4 : 8;
        }
    }
    return w;
}

static uint16_t rswidth(int strid)
{
    static dec64_t dec64;
    uint16_t w = 0;
    char c;

    dec64_init(&dec64, strid);
    while ((c = dec64_getchar(&dec64)) != '\0') {
        if (c >= ' ') {
            w += (c == ' ') ? 4 : 8;
        }
    }
    return w;
}

static void centermsg(uint8_t y, const char *s)
{
    messf(160 - (swidth(s) >> 1), y, ID_S, s);
}

static void centermsgr(uint8_t y, int strid)
{
    messf(160 - (rswidth(strid) >> 1), y, ID_R, strid);
}

void setup_title(int strid)
{
    centermsgr(0x20, strid);
}

void setup_report(int strid)
{
    centermsgr(220, strid);
    delay(2 * 50);
}

static int is_item_visible(const mitem_t *ip)
{
    if (((ip->attrs & M_ATTR_DEVONLY) && !(gd->cheats & CHEAT_DEVMENUS)) ||
        ((ip->attrs & M_ATTR_APPONLY) && gd->idata.from_bootdisk)) {
        return 0;
    } else {
        return 1;
    }
}

static void menu_run(const menu_t *mp, int name)
{
    const mitem_t *ip;
    uint8_t n;
    int i;

    do {
        scrclr();
        setup_title(name);

        if (mp->func != NULL) {
            mp->func();
        }
        gd->curser_y = 0x48 + mp->yoffs;
        for (i = 0; i < mp->nitems; i++) {
            if (is_item_visible(&mp->items[i])) {
                messf(48, gd->curser_y + 10,
                    ID_MENU_ENTRY,
                    '1' + i, mp->items[i].name);
            }
        }
        messf(48, gd->curser_y + 20, ID_BACK);
        do {
            n = setup_getnumkey(mp->nitems);
            ip = &mp->items[n - 1];
        } while (n != 0 && !is_item_visible(ip));
        if (n > 0) {
            if (ip->attrs & M_ATTR_MENU) {
                menu_run(ip->menu, ip->name);
            } else {
                ip->func(n);
            }
        }
    } while (n != 0);
}

#define CFG_CONTROLLER_STR_GEN(a) #a,
static const char *controller_names[] = {
    CFG_CONTROLLER_GENERATOR(CFG_CONTROLLER_STR_GEN)
};

void setup_show_keys(uint8_t y)
{
    const uint8_t *kb = cfg.kbindings[cfg.controller];

    messf(16, y,
        ID_PRIMARY_CONTROLLER,
        controller_names[cfg.controller]);
    switch (cfg.controller) {
    case CFG_CONTROLLER_KEYBOARD:
        messf(16, gd->curser_y,
            ID_CONTROLLER_KEYBOARD,
            kb[KB_UP], kb[KB_FIRE],
            kb[KB_DOWN], kb[KB_SMARTBOMB],
            kb[KB_THRUST], kb[KB_REVERSE],
            kb[KB_HYPERSPACE]
            );
        break;

    case CFG_CONTROLLER_JOYSTICK:
        messf(16, gd->curser_y,
            ID_CONTROLLER_JOYSTICK,
            kb[KB_SMARTBOMB], kb[KB_HYPERSPACE]
            );
        break;

    case CFG_CONTROLLER_JOYPAD:
        messf(16, gd->curser_y,
            ID_CONTROLLER_JOYPAD,
            kb[KB_FIRE], kb[KB_SMARTBOMB],
            kb[KB_HYPERSPACE]
            );
        break;
    }
}

struct file_wrapper_args {
    int rep;
    int (*func)(void);
};

static void do_file_wrapper(void *args)
{
    struct file_wrapper_args *ap = args;
    int r, n;

    do {
        r = ap->func();
        if (r == 0) {
            if (ap->rep != 0) {
                setup_report(ap->rep);
            }
            return;
        }
        scrclr();
        setup_title(ID_FILE_ERROR);
        if (gd->file_err == FILE_ERR_OSERR) {
            messf(0x14 << 1, 0x60, ID_OS_ERROR,
                file_errs[gd->file_err], gd->gemdos_errno);
        } else {
            centermsg(0x40, file_errs[gd->file_err]);
        }
        messf(0x18 << 1, 0x90,
            ID_FILE_ERROR_TRY_AGAIN
            );
        n = setup_getnumkey(2);
        if (n == 2) {
            do_change_dir(NULL);
        }
    } while (n != 0);
}

void file_wrapper(int rep, int (*func)(void))
{
    static struct file_wrapper_args args;

    args.rep = rep;
    args.func = func;
    sys_wrap_os(do_file_wrapper, &args);
}

static void options_display(void)
{
    messf(16, 0x46,
        ID_OPTIONS_DISPLAY,
        cfg.autosave_hst ? ID_ON : ID_OFF,
        (cfg.setup_font == CFG_FONT_DEFENDER) ?  ID_DEFENDER : ID_ATARI
        );
}

static void toggle_auto_save(uint8_t id)
{
    cfg.autosave_hst = !cfg.autosave_hst;
}

static void toggle_setup_font(uint8_t id)
{
    cfg.setup_font = !cfg.setup_font;
    gd->font = (cfg.setup_font == CFG_FONT_ATARI) ? &atari_font : &defender_font;
}

static const mitem_t options_items[] = {
    ITEM_FUNC(ID_AUTO_SAVE_HIGH_SCORES, toggle_auto_save),
    ITEM_FUNC(ID_SETUP_MENU_FONT,       toggle_setup_font),
};
MENU_SPEC(options_menu, options_items, options_display, 48);

static void change_controller(uint8_t id)
{
    if (gd->has_joypad) {
        if (++cfg.controller == CFG_CONTROLLER__NUM) {
            cfg.controller = CFG_CONTROLLER_KEYBOARD;
        }
    } else {
        if (++cfg.controller == CFG_CONTROLLER_JOYPAD) {
            cfg.controller = CFG_CONTROLLER_KEYBOARD;
        }
    }
}

static uint8_t defn_key(int strid)
{
    uint8_t k;

    blkclr(16, 200, 304, 8);
    messf(160 - ((100 + rswidth(strid)) >> 1), 200,
            ID_PRESS_KEY_FOR, strid);
    k = getkey();
    delay(12);
    return k;
}

static void define_keys(uint8_t id)
{
    uint8_t *kb = cfg.kbindings[cfg.controller];

    switch (cfg.controller) {
    case CFG_CONTROLLER_KEYBOARD:
        kb[KB_UP] = defn_key(ID_UP);
        kb[KB_DOWN] = defn_key(ID_DOWN);
        kb[KB_THRUST] = defn_key(ID_THRUST);
        kb[KB_FIRE] = defn_key(ID_FIRE);
        kb[KB_REVERSE] = defn_key(ID_REVERSE);
        kb[KB_SMARTBOMB] = defn_key(ID_SMART_BOMB);
        kb[KB_HYPERSPACE] = defn_key(ID_HYPERSPACE);
        break;

    case CFG_CONTROLLER_JOYSTICK:
        kb[KB_SMARTBOMB] = defn_key(ID_SMART_BOMB);
        kb[KB_HYPERSPACE] = defn_key(ID_HYPERSPACE);
        break;

    case CFG_CONTROLLER_JOYPAD:
        kb[KB_FIRE] = defn_key(ID_FIRE);
        kb[KB_SMARTBOMB] = defn_key(ID_SMART_BOMB);
        kb[KB_HYPERSPACE] = defn_key(ID_HYPERSPACE);
        break;
    }
}

static void keys_restore_defaults(uint8_t id)
{
    reset_kbindings();
    setup_report(ID_DEFAULT_KEY_BINDINGS_RESTORED);
}

static void edit_keys_display(void)
{
    setup_show_keys(0x46);
}

static const mitem_t key_bindings_items[] = {
    ITEM_FUNC(ID_CHANGE_CONTROLLER,     change_controller),
    ITEM_FUNC(ID_CHANGE_KEY_BINDINGS,   define_keys),
    ITEM_FUNC(ID_RESTORE_DEFAULTS,      keys_restore_defaults),
};
MENU_SPEC(key_bindings_menu, key_bindings_items, edit_keys_display, 48);

static void save_config(uint8_t id)
{
    file_wrapper(ID_CONFIG_SAVED, write_cfg);
}

static void load_config(uint8_t id)
{
    file_wrapper(ID_CONFIG_LOADED, read_cfg);
    gd->font = (cfg.setup_font == CFG_FONT_ATARI) ? &atari_font : &defender_font;
}

static void reset_config(uint8_t id)
{
    reset_all_config();
    setup_report(ID_SETTINGS_RESET);
    gd->font = (cfg.setup_font == CFG_FONT_ATARI) ? &atari_font : &defender_font;
}

static const mitem_t settings_items[] = {
    ITEM_MENU(ID_CONFIGURE_CONTROLS,    &key_bindings_menu),
    ITEM_MENU(ID_CONFIGURE_OPTIONS,     &options_menu),
    ITEM_FUNC(ID_CHANGE_DIRECTORY,      &change_dir),
    ITEM_FUNC(ID_SAVE_CONFIG,           save_config),
    ITEM_FUNC(ID_LOAD_CONFIG,           load_config),
    ITEM_FUNC(ID_RESET_ALL_CONFIG,      reset_config),
};
MENU(settings_menu, settings_items);

static void save_hst(uint8_t id)
{
    file_wrapper(ID_HIGH_SCORES_SAVED, write_hst);
}

static void load_hst(uint8_t id)
{
    file_wrapper(ID_HIGH_SCORES_LOADED, read_hst);
}

static void reset_hst(uint8_t id)
{
    reset_all_hst();
    setup_report(ID_HIGH_SCORES_RESET);
}

static void high_scores_display(void)
{
    centermsgr(50, ID_ALL_TIME);
    centermsgr(60, ID_GREATEST);
    haldt(alltime_hst, 120, 75);
}

static const mitem_t high_score_items[] = {
    ITEM_FUNC(ID_SAVE_HIGH_SCORES,  save_hst),
    ITEM_FUNC(ID_LOAD_HIGH_SCORES,  load_hst),
    ITEM_FUNC(ID_RESET_HIGH_SCORES, reset_hst),
};
MENU_SPEC(high_score_menu, high_score_items, high_scores_display, 80);

static void about(uint8_t id)
{
    const font_t *ofont = gd->font;
    static const uint16_t pages[][2] = {
        { ID_ABOUT_TEXT0, ID_ABOUT_NEXT },
        { ID_ABOUT_TEXT1, ID_ABOUT_PREV_NEXT },
        { ID_ABOUT_TEXT2, ID_ABOUT_PREV },
    };
    int page = 0;
    int n;

    do {
        scrclr();
        setup_title(ID_ABOUT_WILLIAMS_DEFENDER);
        gd->font = &atari_font;
        messf(16, 58, pages[page][0]);
        gd->font = ofont;
        messf(48, 0xc0, pages[page][1]);
        n = setup_getnumkey(2);
        if (page > 0 && n == 1) {
            --page;
        } else if (page < 2 && n == 2) {
            ++page;
        }
    } while (n != 0);
}

static const uint16_t top_talent[] = {
    ID_CREDIT0,     ID_CREDIT1,     ID_CREDIT2,     ID_CREDIT3,
    ID_CREDIT4,     ID_CREDIT5,     ID_CREDIT6,     ID_CREDIT7,
    ID_CREDIT8,     ID_CREDIT9,     ID_CREDIT10,    ID_CREDIT11,
    ID_CREDIT12,    ID_CREDIT13,    ID_CREDIT14,    ID_CREDIT15,
    ID_CREDIT16
};

static void credits(uint8_t id)
{
    const font_t *ofont = gd->font;
    int ntalent = sizeof(top_talent) / sizeof(top_talent[0]);
    int nhalf = ntalent / 2;
    int halfoffs = nhalf + (ntalent & 1);
    int i;

    scrclr();
    setup_title(ID_ITEM_CREDITS);
    gd->font = &atari_font;
    messf(16, 50, ID_CREDITS_TEXT);
    for (i = 0; i < nhalf; i++) {
        messf(32, gd->curser_y,
            ID_CREDIT_EVEN,
            top_talent[i], top_talent[halfoffs + i]
            );
    }
    if (ntalent & 1) {
        messf(32, gd->curser_y,
            ID_CREDIT_ODD,
            top_talent[nhalf]
            );
    }
    gd->font = ofont;
    messf(48, gd->curser_y + 10, ID_BACK);
    setup_getnumkey(0);
}

static void license(uint8_t id)
{
    const font_t *ofont = gd->font;

    scrclr();
    setup_title(ID_LICENSE);
    gd->font = &atari_font;
    messf(16, 58, ID_LICENSE_TEXT);
    gd->font = ofont;
    messf(48, gd->curser_y + 10, ID_BACK);
    setup_getnumkey(0);
}

#define IDATA_MACH_STR_GEN(a) #a,
static const char *idata_mach_str[IDATA_MACH__NUM] = {
    IDATA_MACH_GENERATOR(IDATA_MACH_STR_GEN)
};

#define IDATA_CPU_STR_GEN(a) #a,
static const char *idata_cpu_str[IDATA_CPU__NUM] = {
    IDATA_CPU_GENERATOR(IDATA_CPU_STR_GEN)
};

#define IDATA_VID_STR_GEN(a) #a,
static const char *idata_vid_str[IDATA_VID__NUM] = {
    IDATA_VID_GENERATOR(IDATA_VID_STR_GEN)
};

static const char *idata_features_str(void)
{
    static char features_buf[16];
    char *wp = features_buf;

    wp[1] = '\0';
    if (gd->idata.features & IDATA_FEAT_SDMA) {
        memcpy(wp, " SDMA", 6);
        wp += 5;
    }
    if (gd->idata.features & IDATA_FEAT_ENHJOY) {
        memcpy(wp, " ENHJOY", 8);
    }
    return features_buf + 1;
}

static void sys_info(uint8_t id)
{
    scrclr();
    setup_title(ID_SYSTEM_INFORMATION);
    messf(44, 58,
        ID_SYSTEM_INFO_TEXT,
        version_str, git_rev, build_date,
        !!gd->idata.from_bootdisk,
        gd->tos_version >> 8, gd->tos_version & 0xff,
        gd->gemdos_version & 0xff, gd->gemdos_version >> 8,
        idata_mach_str[gd->idata.mach],
        phystop,
        idata_cpu_str[gd->idata.cpu],
        idata_vid_str[gd->idata.vid],
        idata_features_str(),
        gd->issnddma ? ID_DMA : ID_PSG
        );
    messf(48, gd->curser_y + 20, ID_BACK);
    setup_getnumkey(0);
}

static void quit_program(uint8_t id)
{
    sys_exit();
}

static void dev_mem_info(uint8_t id)
{
    extern char _start[];

    scrclr();
    setup_title(ID_MEMORY_INFORMATION);
    messf(44, 58,
        ID_MEM_INFO_TEXT,
        gd->vid_base,
        heap,
        gd->vid_base - (uint32_t)heap,
        _start,
        (uint32_t)heap - (uint32_t)_start,
        gd->vid_base - (uint32_t)_start,
        MEM_USED_PSG, MEM_USED_DMA
        );
    messf(48, gd->curser_y + 20, ID_BACK);
    setup_getnumkey(0);
}

static void dev_cheats_display(void)
{
    messf(16, 0x46,
        ID_CHEATS_DISPLAY,
        (gd->cheats & CHEAT_INVINCIBLE) ? ID_ON : ID_OFF,
        (gd->cheats & CHEAT_SCORE) ? ID_ON : ID_OFF,
        (gd->cheats & CHEAT_SBMAGIC) ? ID_ON : ID_OFF
        );
}

static void dev_cheats_toggle(uint8_t id)
{
    switch (id) {
    case 1: gd->cheats ^= CHEAT_INVINCIBLE; break;
    case 2: gd->cheats ^= CHEAT_SCORE; break;
    case 3: gd->cheats ^= CHEAT_SBMAGIC; break;
    }
}

static const mitem_t dev_cheats_menu_items[] = {
    ITEM_FUNC(ID_INVINCIBILITY,         dev_cheats_toggle),
    ITEM_FUNC(ID_SCORE_CONTROL,         dev_cheats_toggle),
    ITEM_FUNC(ID_MEGA_SMART_BOMBS,      dev_cheats_toggle),
};
MENU_SPEC(dev_cheats_menu, dev_cheats_menu_items, dev_cheats_display, 80);

static const mitem_t dev_menu_items[] = {
    ITEM_FUNC(ID_MEMORY_INFO,           dev_mem_info),
    ITEM_MENU(ID_CHEAT_ENABLES,         &dev_cheats_menu),
};
MENU(dev_menu, dev_menu_items);

static const mitem_t root_items[] = {
    ITEM_MENU(ID_CONFIGURE_SETTINGS,        &settings_menu),
    ITEM_MENU(ID_MANAGE_HIGH_SCORES,        &high_score_menu),
    ITEM_FUNC(ID_ABOUT,                     about),
    ITEM_FUNC(ID_ITEM_CREDITS,              credits),
    ITEM_FUNC(ID_LICENSE,                   license),
    ITEM_FUNC(ID_SYSTEM_INFORMATION,        sys_info),
    ITEM_FUNC_APPONLY(ID_QUIT_PROGRAM,      quit_program),
    ITEM_MENU_DEVONLY(ID_DEVELOPER_MENU,    &dev_menu),
};
MENU(root_menu, root_items);

void enter_setup(void)
{
    uint8_t flags;

    gncide();
    gd->status = 0xff;
    flags = irq_thread_disable();
    expu();
    pcram[1] = 0xa5;
    colr_apply();
    gd->font = (cfg.setup_font == CFG_FONT_ATARI) ? &atari_font : &defender_font;
    menu_run(&root_menu, ID_DEFENDER_SETUP);
    gd->font = &defender_font;
    irq_thread_restore(flags);
    amodes();
}

void setupsw(void)
{
    if (gd->status & ST_GAMEOV) {
        enter_setup();
    }
    sucide();
}

