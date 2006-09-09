## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Sep  9 21:22:11 2006.
# $Id: Makefile.am,v 1.4 2006/09/09 12:52:29 sian Exp $

if USE_AVCODEC
noinst_LTLIBRARIES = libavutil.la
endif
INCLUDES = -I$(top_srcdir)/lib/avcodec -I$(top_srcdir)
CFLAGS_POST = -Wno-error -Wno-unused
DEFS = -prefer-non-pic -DHAVE_AV_CONFIG_H -Dasm=__asm -Dvolatile=__volatile

noinst_HEADERS = \
	adler32.h \
	avutil.h \
	bswap.h \
	common.h \
	crc.h \
	integer.h \
	internal.h \
	intfloat_readwrite.h \
	lls.h \
	log.h \
	mathematics.h \
	md5.h \
	rational.h \
	softfloat.h \
	x86_cpu.h

libavutil_la_SOURCES = \
	adler32.c \
	crc.c \
	integer.c \
	intfloat_readwrite.c \
	lls.c \
	log.c \
	mathematics.c \
	md5.c \
	mem.c \
	rational.c \
	softfloat.c
