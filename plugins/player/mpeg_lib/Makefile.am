## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:36:28 2003.
# $Id: Makefile.am,v 1.6 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/mpeg_lib

libdir = $(enfle_player_plugindir)

if HAVE_MPEG_LIB
lib_LTLIBRARIES = player_mpeg_lib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mpeg_lib_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_mpeg_lib_la_CFLAGS = $(MORE_CFLAGS)
player_mpeg_lib_la_SOURCES = mpeg_lib.c
player_mpeg_lib_la_LIBADD = @LIBADD_MPEG_LIB@
