## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec  3 14:00:36 2000.
# $Id: Makefile.am,v 1.2 2000/12/03 08:40:03 sian Exp $

noinst_LTLIBRARIES = libenfleX11.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

sources = \
	x11.c x11.h \
	x11window.c x11window.h \
	x11ximage.c x11ximage.h

if USE_NASM
libenfleX11_la_SOURCES = $(sources) bgra32to16_mmx.asm
else
libenfleX11_la_SOURCES = $(sources)
endif

libenfleX11_la_LIBADD = -lX11 @LIBADD_SHM@

SUFFIXES=.asm
.asm.lo:
	nasm -f elf -o $(*F).o $<
	ln -s $(*F).o $(*F).lo 
