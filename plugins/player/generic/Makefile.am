## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Feb 13 08:49:34 2004.
# $Id: Makefile.am,v 1.1 2004/02/14 05:22:46 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/generic

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_generic.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_generic_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_generic_la_CFLAGS = $(MORE_CFLAGS)
player_generic_la_SOURCES = generic.c
#player_generic_la_LIBADD = \
#	@LIBADD_PTHREAD@ \
#	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
