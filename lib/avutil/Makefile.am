## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Nov 27 01:25:06 2005.
# $Id: Makefile.am,v 1.1 2005/12/27 15:09:59 sian Exp $

if USE_AVCODEC
noinst_LTLIBRARIES = libavutil.la
endif
INCLUDES = -I$(top_srcdir)/lib/avcodec -I$(top_srcdir)
CFLAGS_POST = -Wno-error -Wno-unused
DEFS = -prefer-non-pic -DHAVE_AV_CONFIG_H -Dasm=__asm -Dvolatile=__volatile

noinst_HEADERS = \
	avutil.h \
	bswap.h \
	common.h \
	integer.h \
	intfloat_readwrite.h \
	mathematics.h \
	rational.h

libavutil_la_SOURCES = \
	integer.c \
	intfloat_readwrite.c \
	mathematics.c \
	rational.c
