## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2002/08/03 05:08:40 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = effect/rotate

libdir = $(enfle_effect_plugindir)

lib_LTLIBRARIES = effect_rotate.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

effect_rotate_la_LDFLAGS = $(MORE_LDFLAGS) -module
effect_rotate_la_CFLAGS = $(MORE_CFLAGS)
effect_rotate_la_SOURCES = rotate.c
