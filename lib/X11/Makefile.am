## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Feb 15 03:51:38 2002.
# $Id: Makefile.am,v 1.6 2002/02/14 19:20:25 sian Exp $

noinst_LTLIBRARIES = libenfleX11.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib
EXTRA_DIST =  bgra32to16_mmx.s  bgra32to16_mmx.asm

sources = \
	x11.c x11.h \
	x11window.c x11window.h \
	x11ximage.c x11ximage.h \
	rgbparse.c rgbparse.h

if USE_MMX
libenfleX11_la_SOURCES = $(sources) bgra32to16_mmx.s
#libenfleX11_la_SOURCES = $(sources) bgra32to16_mmx.asm
else
libenfleX11_la_SOURCES = $(sources)
endif

libenfleX11_la_LIBADD = -lX11 @LIBADD_SHM@ @LIBADD_XV@

#SUFFIXES=.asm
#.asm.o:
#	nasm -f elf -o $(*F).o $<
#	ln -s $(*F).o $(*F).lo
