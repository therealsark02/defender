#!/usr/bin/perl -w

#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

while (<>) {
    chop;
    s/;/\/\//;
    s/section\s+code,code/.text/;
    s/^([a-zA-Z0-9_]+):\s+equ\s+([0-9]+)/\t.equ $1, $2/;
    s/dc\.b/.byte/;
    s/even/.align 2/;
    s/([da][0-7])/%$1/g;
    s/sp/%sp/g;
    s/pc/%pc/g;
    s/\$/0x/g;
    s/xdef/.globl/;
    s/\/\/.*//;
    s/\s+$//;
    print $_ . "\n" if !/^$/;
}
print STDOUT '#include "../elfattr.S"' . "\n";
