## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2004/01/30 12:38:30 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = audiodecoder/mad

libdir = $(enfle_audiodecoder_plugindir)

lib_LTLIBRARIES = audiodecoder_mad.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audiodecoder_mad_la_LDFLAGS = $(MORE_LDFLAGS) -module
audiodecoder_mad_la_CFLAGS = $(MORE_CFLAGS)
audiodecoder_mad_la_SOURCES = mad.c
audiodecoder_mad_la_LIBADD = $(top_builddir)/lib/mad/libenflemad.la
