## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/27 17:42:03 sian Exp $

libdir = $(enfle_saver_plugindir)

lib_LTLIBRARIES = saver_jpeg.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

saver_jpeg_la_LDFLAGS = -avoid-version -module
saver_jpeg_la_SOURCES = jpeg.c
saver_jpeg_la_LIBADD = -ljpeg
