## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:52 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_streamer_plugindir)

if HAVE_BZLIB
lib_LTLIBRARIES = streamer_bz2.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

streamer_bz2_la_LDFLAGS = -avoid-version -module
streamer_bz2_la_SOURCES = bz2.c bz2.h
streamer_bz2_la_LIBADD = @LIBADD_BZLIB@
