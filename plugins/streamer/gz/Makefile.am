## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Sep 30 06:45:16 2000.
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

if HAVE_ZLIB
lib_LTLIBRARIES = streamer_gz.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

streamer_gz_la_LDFLAGS = -module
streamer_gz_la_SOURCES = gz.c
streamer_gz_la_LIBADD = @LIBADD_ZLIB@
