## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:02 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_loader_plugindir)

if HAVE_JPEG
lib_LTLIBRARIES = loader_jpeg.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_jpeg_la_LDFLAGS = -avoid-version -module
loader_jpeg_la_SOURCES = jpeg.c
loader_jpeg_la_LIBADD = @LIBADD_JPEG@
