## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/10/28 19:06:20 sian Exp $

noinst_LTLIBRARIES = libdllloader.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils

libdllloader_la_SOURCES = \
	windef.h w32api.h mm.h export.h \
	pe_image.c pe_image.h pe.h \
	module.c module.h \
	kernel32.c kernel32.h \
	user32.c user32.h \
	msvcrt.c msvcrt.h
