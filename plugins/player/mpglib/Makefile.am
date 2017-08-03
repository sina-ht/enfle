## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:36:33 2003.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/mpglib

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_mpglib.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mpglib_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_mpglib_la_CFLAGS = $(MORE_CFLAGS)
player_mpglib_la_SOURCES = mpglib.c
player_mpglib_la_LIBADD = $(top_builddir)/lib/mpglib/libenflempglib.la @LIBADD_PTHREAD@
