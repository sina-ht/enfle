## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Sep  3 01:36:04 2001.
# $Id: Makefile.am,v 1.4 2001/09/03 00:33:26 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_LIBDIVXDECORE
lib_LTLIBRARIES = player_opendivx.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_opendivx_la_LDFLAGS = -avoid-version -module
player_opendivx_la_SOURCES = opendivx.c
player_opendivx_la_LIBADD = \
	@LIBADD_PTHREAD@ \
	@LIBADD_DIVXDECORE@ \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
