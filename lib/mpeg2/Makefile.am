## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  3 00:07:04 2004.
# $Id: Makefile.am,v 1.9 2004/01/03 10:16:24 sian Exp $

noinst_LIBRARIES = libenflempeg2.a
INCLUDES = -I$(top_srcdir)
##CFLAGS_POST = -O3 -fomit-frame-pointer -fno-common -mcpu=pentiumpro

libenflempeg2_a_SOURCES = \
	alloc.c \
	decode.c \
	idct_alpha.c \
	motion_comp.c \
	motion_comp_mmx.c \
	slice.c \
	cpu_accel.c \
	header.c \
	idct_altivec.c \
	motion_comp_alpha.c \
	motion_comp_vis.c \
	vlc.h \
	cpu_state.c \
	idct.c \
	idct_mmx.c \
	motion_comp_altivec.c \
	mpeg2_internal.h \
	mpeg2.h \
	attributes.h \
	mmx.h
