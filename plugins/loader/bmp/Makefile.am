## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:04 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_bmp.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_bmp_la_LDFLAGS = -avoid-version -module
loader_bmp_la_SOURCES = bmp.c
