## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Wed Nov  5 01:07:46 2003.
# $Id: Makefile.am,v 1.1 2003/11/17 13:56:33 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/divx

libdir = $(enfle_player_plugindir)

if HAVE_LIBDIVXDECORE
lib_LTLIBRARIES = player_divx.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_divx_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_divx_la_CFLAGS = $(MORE_CFLAGS)
player_divx_la_SOURCES = divx.c
player_divx_la_LIBADD = \
	@LIBADD_PTHREAD@ \
	@LIBADD_DIVXDECORE@ \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
