## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 16:01:01 2002.
# $Id: Makefile.am,v 1.5 2002/08/03 05:08:37 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/ungif

libdir = $(enfle_player_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = player_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_ungif_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_ungif_la_CFLAGS = $(MORE_CFLAGS)
player_ungif_la_SOURCES = ungif.c
player_ungif_la_LIBADD = @LIBADD_UNGIF@
