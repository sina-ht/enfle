## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 15:59:58 2002.
# $Id: Makefile.am,v 1.5 2002/08/03 05:08:38 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/opendivx

libdir = $(enfle_player_plugindir)

if HAVE_LIBDIVXDECORE
lib_LTLIBRARIES = player_opendivx.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_opendivx_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_opendivx_la_CFLAGS = $(MORE_CFLAGS)
player_opendivx_la_SOURCES = opendivx.c
player_opendivx_la_LIBADD = \
	@LIBADD_PTHREAD@ \
	@LIBADD_DIVXDECORE@ \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
