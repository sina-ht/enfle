## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Sep 22 13:26:11 2000.
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

lib_LTLIBRARIES = loader_png.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_png_la_LDFLAGS = -module
loader_png_la_SOURCES = png.c
loader_png_la_LIBADD = @LIBADD_PNG@
