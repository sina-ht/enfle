## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:54 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_streamer_plugindir)

if HAVE_ZLIB
lib_LTLIBRARIES = streamer_gz.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

streamer_gz_la_LDFLAGS = -avoid-version -module
streamer_gz_la_SOURCES = gz.c
streamer_gz_la_LIBADD = @LIBADD_ZLIB@
