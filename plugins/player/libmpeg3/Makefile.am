## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:36 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_LIBMPEG3
lib_LTLIBRARIES = player_libmpeg3.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg3_la_LDFLAGS = -avoid-version -module
player_libmpeg3_la_SOURCES = libmpeg3.c
player_libmpeg3_la_LIBADD = @LIBADD_LIBMPEG3@
