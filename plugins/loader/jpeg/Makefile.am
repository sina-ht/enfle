## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:10 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_loader_plugindir)

if HAVE_JPEG
lib_LTLIBRARIES = loader_jpeg.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_jpeg_la_LDFLAGS = -avoid-version -module
loader_jpeg_la_SOURCES = jpeg.c
loader_jpeg_la_LIBADD = @LIBADD_JPEG@
