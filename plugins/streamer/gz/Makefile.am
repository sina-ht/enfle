## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:46 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_streamer_plugindir)

if HAVE_ZLIB
lib_LTLIBRARIES = streamer_gz.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

streamer_gz_la_LDFLAGS = -avoid-version -module
streamer_gz_la_SOURCES = gz.c
streamer_gz_la_LIBADD = @LIBADD_ZLIB@
