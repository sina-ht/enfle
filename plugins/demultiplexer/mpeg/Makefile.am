## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2004/02/14 05:22:15 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = demultiplexer/mpeg

libdir = $(enfle_demultiplexer_plugindir)

lib_LTLIBRARIES = demultiplexer_mpeg.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

demultiplexer_mpeg_la_LDFLAGS = $(MORE_LDFLAGS) -module
demultiplexer_mpeg_la_CFLAGS = $(MORE_CFLAGS)
demultiplexer_mpeg_la_SOURCES = \
	mpeg.c demultiplexer_mpeg.h
