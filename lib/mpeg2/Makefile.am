## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Jun 25 02:26:11 2001.
# $Id: Makefile.am,v 1.6 2001/06/24 17:27:59 sian Exp $

noinst_LIBRARIES = libenflempeg2.a
INCLUDES = -I$(top_srcdir)

libenflempeg2_a_SOURCES = \
	decode.c \
	header.c \
	idct.c idct_mlib.c idct_mmx.c \
	motion_comp.c motion_comp_mlib.c motion_comp_mmx.c \
	mpeg2.h mpeg2_internal.h mm_accel.h mmx.h \
	slice.c \
	stats.c \
	vlc.h \
	attributes.h \
	yuv2rgb.c yuv2rgb.h yuv2rgb_mlib.c yuv2rgb_mmx.c \
	video_out.c video_out.h video_out_internal.h \
	cpu_accel.c
