## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:03 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_png.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_png_la_LDFLAGS = -avoid-version -module
loader_png_la_SOURCES = png.c
loader_png_la_LIBADD = @LIBADD_PNG@
