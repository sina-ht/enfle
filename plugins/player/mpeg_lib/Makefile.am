## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:26 2001.
# $Id: Makefile.am,v 1.4 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_MPEG_LIB
lib_LTLIBRARIES = player_mpeg_lib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mpeg_lib_la_LDFLAGS = -avoid-version -module
player_mpeg_lib_la_SOURCES = mpeg_lib.c
player_mpeg_lib_la_LIBADD = @LIBADD_MPEG_LIB@
