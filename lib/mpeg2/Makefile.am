## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue Feb 20 01:39:15 2001.
# $Id: Makefile.am,v 1.2 2001/02/19 16:39:11 sian Exp $

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
	video_out.c video_out.h video_out_internal.h

#libenflempeg2_la_LIBADD = -lm
