## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = saver/jpeg

libdir = $(enfle_saver_plugindir)

if HAVE_JPEG
lib_LTLIBRARIES = saver_jpeg.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

saver_jpeg_la_LDFLAGS = $(MORE_LDFLAGS) -module
saver_jpeg_la_CFLAGS = $(MORE_CFLAGS)
saver_jpeg_la_SOURCES = jpeg.c
saver_jpeg_la_LIBADD = @LIBADD_JPEG@
