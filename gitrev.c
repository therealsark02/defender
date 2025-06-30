/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

/*
 * GIT repo version and human-readable software version.
 */

#define _STR(x)  #x
#define STR(x)  _STR(x)

const char git_rev[] = STR(GITREV);
const char build_date[] = STR(BUILDDATE);
const char version_str[] = "1.01 RELEASE";

