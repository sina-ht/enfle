## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Sep 22 13:26:15 2000.
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

if HAVE_JPEG
lib_LTLIBRARIES = loader_jpeg.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_jpeg_la_LDFLAGS = -module
loader_jpeg_la_SOURCES = jpeg.c
loader_jpeg_la_LIBADD = @LIBADD_JPEG@
