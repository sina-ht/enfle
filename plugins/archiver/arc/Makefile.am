## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Oct  5 23:01:34 2002.
# $Id: Makefile.am,v 1.1 2002/10/05 17:16:41 sian Exp $

libdir = $(enfle_archiver_plugindir)

if HAVE_LIBARC
lib_LTLIBRARIES = archiver_arc.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

archiver_arc_la_LDFLAGS = -avoid-version -module
archiver_arc_la_SOURCES = arc.c
archiver_arc_la_LIBADD = @LIBADD_LIBARC@
