## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Feb 22 01:29:30 2007.
# $Id: Makefile.am,v 1.10 2007/02/21 17:07:19 sian Exp $

if USE_AVCODEC
noinst_LTLIBRARIES = libavutil.la
endif
INCLUDES = -I$(top_srcdir)/lib/avcodec -I$(top_srcdir)
CFLAGS_POST = -Wno-error -Wno-unused
DEFS = -prefer-non-pic -DHAVE_AV_CONFIG_H -Dasm=__asm -Dvolatile=__volatile

noinst_HEADERS = \
	aes.h \
	adler32.h \
	avutil.h \
	base64.h \
	bswap.h \
	common.h \
	crc.h \
	fifo.h \
	integer.h \
	internal.h \
	intfloat_readwrite.h \
	intreadwrite.h \
	lls.h \
	log.h \
	lzo.h \
	mathematics.h \
	md5.h \
	random.h \
	rational.h \
	softfloat.h \
	tree.h \
	x86_cpu.h

# softfloat.c
libavutil_la_SOURCES = \
	aes.c \
	adler32.c \
	base64.c \
	crc.c \
	fifo.c \
	integer.c \
	intfloat_readwrite.c \
	lls.c \
	log.c \
	lzo.c \
	mathematics.c \
	md5.c \
	mem.c \
	random.c \
	rational.c \
	tree.c
