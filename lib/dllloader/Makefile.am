## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.4 2001/10/10 14:43:57 sian Exp $

if USE_SPI
noinst_LTLIBRARIES = libdllloader.la
endif
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

libdllloader_la_SOURCES = \
	windef.h w32api.h export.h \
	mm.c mm.h \
	pe_image.c pe_image.h pe.h \
	module.c module.h \
	kernel32.c kernel32.h \
	user32.c user32.h \
	advapi32.c advapi32.h \
	msvcrt.c msvcrt.h \
	borlndmm.c borlndmm.h
