## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan 27 22:36:55 2001.
# $Id: Makefile.am,v 1.2 2001/01/28 03:48:02 sian Exp $

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_opendivx.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_opendivx_la_LDFLAGS = -avoid-version -module
player_opendivx_la_SOURCES = opendivx.c libavi.c libavi.h libriff.c libriff.h
player_opendivx_la_LIBADD = $(top_builddir)/lib/divxdecore/libdivxdecore.la @LIBADD_DIVXDECORE@
