## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2001/01/06 23:52:52 sian Exp $

if USE_SPI
noinst_LTLIBRARIES = libdllloader.la
endif
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

libdllloader_la_SOURCES = \
	windef.h w32api.h mm.h export.h \
	pe_image.c pe_image.h pe.h \
	module.c module.h \
	kernel32.c kernel32.h \
	user32.c user32.h \
	advapi32.c advapi32.h \
	msvcrt.c msvcrt.h \
	borlndmm.c borlndmm.h
