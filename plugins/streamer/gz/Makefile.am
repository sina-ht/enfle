## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:35:36 2003.
# $Id: Makefile.am,v 1.5 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = streamer/gz

libdir = $(enfle_streamer_plugindir)

if HAVE_ZLIB
lib_LTLIBRARIES = streamer_gz.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

streamer_gz_la_LDFLAGS = $(MORE_LDFLAGS) -module
streamer_gz_la_CFLAGS = $(MORE_CFLAGS)
streamer_gz_la_SOURCES = gz.c
streamer_gz_la_LIBADD = @LIBADD_ZLIB@
