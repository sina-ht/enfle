## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Sep 22 13:23:10 2000.
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

if HAVE_BZLIB
lib_LTLIBRARIES = streamer_bz2.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

streamer_bz2_la_LDFLAGS = -module
streamer_bz2_la_SOURCES = bz2.c bz2.h
streamer_bz2_la_LIBADD = @LIBADD_BZLIB@
