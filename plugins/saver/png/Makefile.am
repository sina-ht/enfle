## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2001/04/24 16:42:20 sian Exp $

libdir = $(enfle_saver_plugindir)

lib_LTLIBRARIES = saver_png.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

saver_png_la_LDFLAGS = -avoid-version -module
saver_png_la_SOURCES = png.c
saver_png_la_LIBADD = -lpng
