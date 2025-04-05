/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * Timer handling.
 */

#include "gd.h"
#include "mfp.h"
#include "timers.h"

void timers_reset_all(void)
{
    writeb(MFP_TCRA, 0);
    writeb(MFP_TCRB, 0);
    // disable all interrupts
    writeb(MFP_IERA, 0);
    writeb(MFP_IERB, 0);
    writeb(MFP_IMRA, 0);
    writeb(MFP_IMRB, 0);
    writeb(MFP_IPRA, 0);
    writeb(MFP_IPRB, 0);
}

