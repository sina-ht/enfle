## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2004/03/11 15:11:34 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = demultiplexer/ogg

libdir = $(enfle_demultiplexer_plugindir)

if HAVE_OGG
lib_LTLIBRARIES = demultiplexer_ogg.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

demultiplexer_ogg_la_LDFLAGS = $(MORE_LDFLAGS) -module
demultiplexer_ogg_la_CFLAGS = $(MORE_CFLAGS)
demultiplexer_ogg_la_SOURCES = \
	ogg.c demultiplexer_ogg.h
demultiplexer_ogg_la_LIBADD = @LIBADD_OGG@ $(top_builddir)/lib/enfle/libenfle.la
