## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Mar  4 09:51:29 2001.
# $Id: Makefile.am,v 1.3 2001/03/04 17:10:41 sian Exp $

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_libmpeg2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg2_la_LDFLAGS = -avoid-version -module
player_libmpeg2_la_SOURCES = libmpeg2.c libmpeg2_vo.c libmpeg2_vo.h
player_libmpeg2_la_LIBADD = \
	-L$(top_builddir)/lib/mpeg2 -lenflempeg2 \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la \
	@LIBADD_PTHREAD@
