## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Nov 13 23:22:05 2003.
# $Id: Makefile.am,v 1.1 2003/11/17 13:56:48 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/avcodec

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_avcodec.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_avcodec_la_CFLAGS = $(MORE_CFLAGS)
player_avcodec_la_SOURCES = avcodec.c
player_avcodec_la_LIBADD = \
	@LIBADD_PTHREAD@ \
	$(top_builddir)/lib/avcodec/libavcodec.la \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
