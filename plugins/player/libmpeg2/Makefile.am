## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue Feb 20 22:01:19 2001.
# $Id: Makefile.am,v 1.2 2001/02/20 13:54:59 sian Exp $

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_libmpeg2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg2_la_LDFLAGS = -avoid-version -module
player_libmpeg2_la_SOURCES = libmpeg2.c libmpeg2_vo.c libmpeg2_vo.h
player_libmpeg2_la_LIBADD = \
	$(top_builddir)/lib/mpeg2/libenflempeg2.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la \
	@LIBADD_PTHREAD@
