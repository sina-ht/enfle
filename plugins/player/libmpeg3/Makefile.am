## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 15:56:42 2002.
# $Id: Makefile.am,v 1.4 2002/08/03 05:08:39 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/libmpeg3

libdir = $(enfle_player_plugindir)

if HAVE_LIBMPEG3
lib_LTLIBRARIES = player_libmpeg3.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg3_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_libmpeg3_la_CFLAGS = $(MORE_CFLAGS)
player_libmpeg3_la_SOURCES = libmpeg3.c
player_libmpeg3_la_LIBADD = @LIBADD_LIBMPEG3@
