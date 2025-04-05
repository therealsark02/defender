/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Player handling (on-screen positioning, direction, thrust)
 */

#include "gd.h"
#include "screen.h"
#include "player.h"

// called from irq to update the player state
void player(void)
{
    int32_t bgdelt, pcx;
    uint16_t base;
    int16_t dx;

    if (gd->status & ST_ATTRACT) {
        // attract mode - player controls inactive
        return;
    }

    // x damping
    gd->plaxv -= (gd->plaxv >> 6) & -4;
    if (gd->pia21 & PIA21_THRUST) {
        // accelerating
        gd->plaxv += gd->pladir;
    }

    // velocity mapping x motion
    // If moving in the same direction as facing,
    // establish a target screen position that is
    // 0.5s (32 frames) of current-velocity movement
    // away from the left or right hand "base" x-position
    pcx = PLAXV_FWD_HALF_SEC;       // pcx is plaxv * 32 (s10.6)
    if (gd->pladir >= 0) {
        base = u8_to_u8p8(0x20);    // right-facing x-pos $20.0
        if (pcx < 0) {
            pcx = 0;                // zero if sign opposes
        }
    } else {
        base = u8_to_u8p8(0x70);    // left-facing x-pos $70.0
        if (pcx >= 0) {
            pcx = 0;                // zero if sign opposes
        }
    }
    /* pcx is the target value for plax16 */
    /* the pcx expression is effectively plaxv * 8, in u8.8 (or 0) */
    pcx = base + ((pcx >> 8) & ~0x7f);
    bgdelt = 0;
    
    // dx = distance to pcx
    dx = pcx - gd->plax16;
    if (dx <= s8_to_s8p8(-1)) {
        // too far, move the bg, and target left
        bgdelt = -0x40;             // -1.0 (s10.6)
        pcx = gd->plax16 - u8_to_u8p8(1);   // pcx = cur - 1.0
    } else if (dx > s8_to_s8p8(1)) {
        bgdelt = 0x40;              // bgdelt = +1.0
        pcx = gd->plax16 + u8_to_u8p8(1);   // pcx = cur + 1.0
    }
    gd->plax16 = pcx;
    gd->nplascrx = u8p8_to_screenx(pcx);        // 1px prec
    gd->nplaxc = gd->nplascrx >> 1;             // 2px prec
    gd->bglx = gd->bgl;

    // keep plaxv within -1.00_00 .. +1.00_00
    if (gd->plaxv < -PLAXV_MAX) {
        gd->plaxv = -PLAXV_MAX;
    } else if (gd->plaxv > PLAXV_MAX) {
        gd->plaxv = PLAXV_MAX;
    }

    gd->bgl = (gd->plaxv >> 8) + gd->bgl - bgdelt;
    gd->plabx = gd->bgl + ((gd->plax16 >> 2) & 0xffe0);

    // vertical motion
    if (gd->pia31 & PIA31_UP) {
        // joystick player up
        if ((gd->play16 & 0xff00) <= u8_to_u8p8(YMIN + 1)) {
            // cannot go further up
            return;
        }
        if (gd->playv < 0) {
            // accelerating up
            gd->playv -= PLAYV_STEP;     // playv -= $0.08
            if (gd->playv < -PLAYV_MAX) {
                gd->playv = -PLAYV_MAX;  // cap at -$2.00
            }
        } else {
            // stationary or down-to-up change
            gd->playv = -PLAYV_START;    // playv = -$1.00
        }
    } else if (gd->pia21 & PIA21_DOWN) {
        // joystick player down
        if (gd->play16 >= u8_to_u8p8(YMAX - 2)) {
            // cannot go further down
            return;
        }
        if (gd->playv > 0) {
            // accelerating down
            gd->playv += PLAYV_STEP;       // playv += $0.08
            if (gd->playv > PLAYV_MAX) {
                gd->playv = PLAYV_MAX;  // cap at +$2.00
            }
        } else {
            // stationary or up-to-down change
            gd->playv = PLAYV_START;      // playv = +$1.00
        }
    } else {
        gd->playv = 0;              // stop vertical movement
    }
    gd->play16 += gd->playv;
    gd->nplayc = u8p8_to_u8(gd->play16);
}

static void revx1(void)
{
    gd->revflg = 0;
    sucide();
}

static void rev2(void)
{
    if (gd->pia21 & PIA21_REVERSE) {
        sleep(rev2, 2);
    }
    sleep(revx1, 5);
}

void rev(void)
{
    if (!gd->revflg) {
        ++gd->revflg;
        gd->nplad = -gd->pladir;
        sleep(rev2, 2);
    }
    sucide();
}

