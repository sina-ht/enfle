## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:36 2001.
# $Id: Makefile.am,v 1.1 2001/02/01 16:10:22 sian Exp $

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_libmpeg2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg2_la_LDFLAGS = -avoid-version -module
player_libmpeg2_la_SOURCES = libmpeg2.c
player_libmpeg2_la_LIBADD = $(top_builddir)/lib/mpeg2/libenflempeg2.la @LIBADD_PTHREAD@
