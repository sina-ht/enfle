## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Feb 22 18:06:28 2001.
# $Id: Makefile.am,v 1.3 2001/02/22 17:48:18 sian Exp $

#AM_CFLAGS =

noinst_LTLIBRARIES = libenflempeg2.la
INCLUDES = -I$(top_srcdir)

libenflempeg2_la_SOURCES = \
	decode.c \
	header.c \
	idct.c idct_mlib.c idct_mmx.c \
	motion_comp.c motion_comp_mlib.c motion_comp_mmx.c \
	mpeg2.h mpeg2_internal.h mm_accel.h mmx.h \
	slice.c \
	stats.c \
	vlc.h \
	yuv2rgb.c yuv2rgb.h yuv2rgb_mlib.c yuv2rgb_mmx.c \
	video_out.c video_out.h video_out_internal.h \
	cpu_accel.c

#libenflempeg2_la_LIBADD = -lm
