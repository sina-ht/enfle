## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Jan 29 23:43:22 2001.
# $Id: Makefile.am,v 1.1 2001/01/29 15:10:31 sian Exp $

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_mpglib.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mpglib_la_LDFLAGS = -avoid-version -module
player_mpglib_la_SOURCES = mpglib.c
player_mpglib_la_LIBADD = $(top_builddir)/lib/mpglib/libenflempglib.la @LIBADD_PTHREAD@
