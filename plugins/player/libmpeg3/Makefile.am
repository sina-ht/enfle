## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:40 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_LIBMPEG3
lib_LTLIBRARIES = player_libmpeg3.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/utils

player_libmpeg3_la_LDFLAGS = -avoid-version -module
player_libmpeg3_la_SOURCES = libmpeg3.c
player_libmpeg3_la_LIBADD = @LIBADD_LIBMPEG3@
