## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2002/08/03 05:08:37 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = saver/png

libdir = $(enfle_saver_plugindir)

lib_LTLIBRARIES = saver_png.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

saver_png_la_LDFLAGS = $(MORE_LDFLAGS) -module
saver_png_la_CFLAGS = $(MORE_CFLAGS)
saver_png_la_SOURCES = png.c
saver_png_la_LIBADD = @LIBADD_PNG@
