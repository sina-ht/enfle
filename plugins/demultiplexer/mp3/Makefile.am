## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2004/03/03 15:45:36 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = demultiplexer/mp3

libdir = $(enfle_demultiplexer_plugindir)

lib_LTLIBRARIES = demultiplexer_mp3.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

demultiplexer_mp3_la_LDFLAGS = $(MORE_LDFLAGS) -module
demultiplexer_mp3_la_CFLAGS = $(MORE_CFLAGS)
demultiplexer_mp3_la_SOURCES = mp3.c
