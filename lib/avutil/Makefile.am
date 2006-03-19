## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Mar 19 16:35:04 2006.
# $Id: Makefile.am,v 1.2 2006/03/19 07:35:40 sian Exp $

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
	crc.h \
	integer.h \
	intfloat_readwrite.h \
	mathematics.h \
	rational.h

libavutil_la_SOURCES = \
	crc.c \
	integer.c \
	intfloat_readwrite.c \
	mathematics.c \
	rational.c
