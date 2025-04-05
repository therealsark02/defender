BUILD INSTRUCTIONS
------------------

Envrionment and Tools
---------------------
Defender was built on Ubuntu 22.04 under Windows Subsystem for Linux.

The m68k compiler toolchain is GCC, created by
Crosstool-NG (crosstool-ng.github.io)
commit 0145966e8e4f73843a72733e59263ce3f8c69f2e

Building: m68k-elf-gcc (crosstool-NG 1.26.0.133_0145966) 14.2.0

The x86_64 tools were built using:
gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0

Other tools used:
    Python 3.10.12
    Perl v5.34.0
    Mtools version 4.0.32

Build Procedure
---------------
Pull the liblzg git submodule:
    $ git submodule update --init

At the top level, type:
    $ make CROSS_COMPILE=/path/to/cross-compiler

I have installed the crosstool toolchain under ~/x-tools, so I type, and the
tools have the prefix m68k-elf- (e.g. m68k-elf-gcc), so I type:
    $ make CROSS_COMPILE=~/x-tools/m68k-elf/bin/m68k-elf-

If all goes well you'll have:

defender.st     - A floppy-bootable image
defender.zip    - The same files, but easy to install on a hard disk.

