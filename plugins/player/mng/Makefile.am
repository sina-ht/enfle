## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 15:56:51 2002.
# $Id: Makefile.am,v 1.4 2002/08/03 05:08:39 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/mng

libdir = $(enfle_player_plugindir)

if HAVE_MNG
lib_LTLIBRARIES = player_mng.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mng_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_mng_la_CFLAGS = $(MORE_CFLAGS)
player_mng_la_SOURCES = mng.c
player_mng_la_LIBADD = @LIBADD_MNG@
