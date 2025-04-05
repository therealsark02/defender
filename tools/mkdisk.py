#!/usr/bin/env python3

#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2025 sark02
#

import sys
import struct

def install_bootimg(path, boot):
    with open(boot, "rb") as f:
        bimg = f.read()
    blen = len(bimg)
    if blen > 510:
        raise ValueError("boot image too big")
    sect = bimg + bytearray('\0' * (510 - blen), encoding="UTF-8")
    sum = 0
    for i in range(0, len(sect), 2):
        sum = sum + struct.unpack('>H', sect[i:i+2])[0]
    sect = sect + struct.pack('>H', (0x1234 - sum) & 0xffff)
        
    with open(path, "r+b") as f:
        f.write(sect)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'usage: {sys.argv[0]} bootimg diskfile', file=sys.stderr)
        exit(1)
    bootimg = sys.argv[1]
    diskimg = sys.argv[2]
    install_bootimg(diskimg, bootimg)
