## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:56 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_streamer_plugindir)

if HAVE_BZLIB
lib_LTLIBRARIES = streamer_bz2.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

streamer_bz2_la_LDFLAGS = -avoid-version -module
streamer_bz2_la_SOURCES = bz2.c bz2.h
streamer_bz2_la_LIBADD = @LIBADD_BZLIB@
