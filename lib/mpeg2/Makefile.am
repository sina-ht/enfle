## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan 27 00:35:20 2001.
# $Id: Makefile.am,v 1.1 2001/02/01 01:08:52 sian Exp $

noinst_LTLIBRARIES = libenflempeg2.la
INCLUDES = -I$(top_srcdir)

libenflempeg2_la_SOURCES = \
	debug.c debug.h \
	decode.c \
	header.c header.h \
	idct.c idct.h \
	idct_mlib.c idct_mlib.h \
	idct_mmx.c idct_mmx.h \
	motion_comp.c motion_comp.h \
	motion_comp_mlib.c \
	motion_comp_mmx.c \
	mpeg2.h mpeg2_internal.h \
	oms_accel.h \
	slice.c slice.h \
	stats.c stats.h \
	video_out.h \
	vlc.h

libenflempeg2_la_LIBADD = -lm
