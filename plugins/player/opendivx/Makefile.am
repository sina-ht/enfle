## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Aug 26 08:03:21 2001.
# $Id: Makefile.am,v 1.3 2001/08/26 00:56:57 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_LIBDIVXDECORE
lib_LTLIBRARIES = player_opendivx.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_opendivx_la_LDFLAGS = -avoid-version -module
player_opendivx_la_SOURCES = opendivx.c libavi.c libavi.h libriff.c libriff.h
player_opendivx_la_LIBADD = @LIBADD_DIVXDECORE@
