/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 sark02
 */

	.section .text

// sp+4 -> c
// sp+0 -> rts
	.globl	gemdos_cconout
gemdos_cconout:
	move.l	4(%sp),%d0
	andi.l	#0xff,%d0
	movem.l	%d2/%a2,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#2,-(%sp)
	trap	#1
	addq	#4,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_sversion
gemdos_sversion:
	movem.l	%d2/%a2,-(%sp)
	move.w	#0x30,-(%sp)
	trap	#1
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fcreate
// sp->[0:rts,4:path,8:mode]
gemdos_fcreate:
	movem.l	4(%sp),%d0/%d1
	movem.l	%d2/%a2,-(%sp)
	move.w	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x3c,-(%sp)
	trap	#1
	addq	#8,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fopen
// sp->[0:rts,4:path,8:mode]
gemdos_fopen:
	movem.l	4(%sp),%d0/%d1
	movem.l	%d2/%a2,-(%sp)
	move.w	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x3d,-(%sp)
	trap	#1
	addq	#8,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fclose
// sp->[0:rts,4:fd]
gemdos_fclose:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#0x3e,-(%sp)
	trap	#1
	addq	#4,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fread
// sp->[0:ret,4:fd,8:buf,c:len]
gemdos_fread:
	movem.l	4(%sp),%d0-%d1/%a0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d1,-(%sp)
	move.l	%a0,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#0x3f,-(%sp)
	trap	#1
	lea	12(%sp),%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fwrite
// sp->[0:ret,4:fd,8:buf,c:len]
gemdos_fwrite:
	movem.l	4(%sp),%d0-%d1/%a0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d1,-(%sp)
	move.l	%a0,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#0x40,-(%sp)
	trap	#1
	lea	12(%sp),%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fseek
// sp->[0:ret,4:fd,8:offset,c:whence]
gemdos_fseek:
	movem.l	4(%sp),%d0-%d1/%a0
	movem.l	%d2/%a2,-(%sp)
	move.w	%a0,-(%sp)
	move.w	%d0,-(%sp)
	move.l	%d1,-(%sp)
	move.w	#0x42,-(%sp)
	trap	#1
	lea	10(%sp),%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_super
gemdos_super:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x20,-(%sp)
	trap	#1
	addq.l	#6,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_cconws
gemdos_cconws:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#9,-(%sp)
	trap	#1
	addq.l	#6,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_cnecin
gemdos_cnecin:
	movem.l	%d2/%a2,-(%sp)
	move.w	#8,-(%sp)
	trap	#1
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_pterm
gemdos_pterm:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#0x4c,-(%sp)
	trap	#1
	addq	#4,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_dgetdrv
gemdos_dgetdrv:
	movem.l	%d2/%a2,-(%sp)
	move.w	#0x19,-(%sp)
	trap	#1
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

// sp->[0:ret,4:drv]
	.globl	gemdos_dsetdrv
gemdos_dsetdrv:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#0x0e,-(%sp)
	trap	#1
	addq	#4,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

// sp->[0:ret,4:path,8:driveno]
	.globl	gemdos_dgetpath
gemdos_dgetpath:
	movem.l	4(%sp),%d0-%d1
	movem.l	%d2/%a2,-(%sp)
	move.w	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x47,-(%sp)
	trap	#1
	addq	#8,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

// sp->[0:ret,4:path]
	.globl	gemdos_dsetpath
gemdos_dsetpath:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x3b,-(%sp)
	trap	#1
	addq	#6,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fgetdta
gemdos_fgetdta:
	movem.l	%d2/%a2,-(%sp)
	move.w	#0x2f,-(%sp)
	trap	#1
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

// sp->[0:ret,4:fspec, 8:attrs]
	.globl	gemdos_fsfirst
gemdos_fsfirst:
	movem.l	4(%sp),%d0-%d1
	movem.l	%d2/%a2,-(%sp)
	move.w	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#0x4e,-(%sp)
	trap	#1
	addq	#8,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	gemdos_fsnext
gemdos_fsnext:
	movem.l	%d2/%a2,-(%sp)
	move.w	#0x4f,-(%sp)
	trap	#1
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	xbios_physbase
xbios_physbase:
	movem.l	%d2/%a2,-(%sp)
	move.w	#2,-(%sp)
	trap	#14
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	xbios_logbase
xbios_logbase:
	movem.l	%d2/%a2,-(%sp)
	move.w	#3,-(%sp)
	trap	#14
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	xbios_getrez
xbios_getrez:
	movem.l	%d2/%a2,-(%sp)
	move.w	#4,-(%sp)
	trap	#14
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	and.l	#0xffff,%d0
	rts

	// sp->[0:ret,4:logaddr,8:physaddr,c:res]
	.globl	xbios_setscreen
xbios_setscreen:
	movem.l	4(%sp),%d0-%d1/%a0
	movem.l	%d2/%a2,-(%sp)
	move.w	%a0,-(%sp)
	move.l	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#5,-(%sp)
	trap	#14
	lea	12(%sp),%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	// sp->[0:ret,4:logaddr,8:physaddr,c:res,10:mode]
	.globl	xbios_vsetscreen
xbios_vsetscreen:
	movem.l	4(%sp),%d0-%d1/%a0-%a1
	movem.l	%d2/%a2,-(%sp)
	move.w	%a1,-(%sp)
	move.w	%a0,-(%sp)
	move.l	%d1,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#5,-(%sp)
	trap	#14
	lea	14(%sp),%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	xbios_setpalette
xbios_setpalette:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.l	%d0,-(%sp)
	move.w	#6,-(%sp)
	trap	#14
	addq	#6,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	// sp->[0:ret,4:colornum,8:color]
	.globl	xbios_setcolor
xbios_setcolor:
	movem.l	4(%sp),%d0-%d1
	movem.l	%d2/%a2,-(%sp)
	move.w	%d1,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#7,-(%sp)
	trap	#14
	addq	#6,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

	.globl	xbios_mon_type
xbios_mon_type:
	movem.l	%d2/%a2,-(%sp)
	move.w	#89,-(%sp)
	trap	#14
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	and.l	#0xffff,%d0
	rts
	
	.globl	xbios_vsetmode
xbios_vsetmode:
	move.l	4(%sp),%d0
	movem.l	%d2/%a2,-(%sp)
	move.w	%d0,-(%sp)
	move.w	#88,-(%sp)
	trap	#14
	addq	#4,%sp
	movem.l	(%sp)+,%d2/%a2
	and.l	#0xffff,%d0
	rts

	.globl	xbios_wvbl
xbios_wvbl:
	movem.l	%d2/%a2,-(%sp)
	move.w	#37,-(%sp)
	trap	#14
	addq	#2,%sp
	movem.l	(%sp)+,%d2/%a2
	rts

#include "elfattr.S"
