## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2004/03/31 14:40:06 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = videodecoder/avcodec

libdir = $(enfle_videodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = videodecoder_avcodec.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

videodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module -export-symbols-regex 'plugin_*'
videodecoder_avcodec_la_CFLAGS = $(MORE_CFLAGS)
videodecoder_avcodec_la_SOURCES = avcodec.c
videodecoder_avcodec_la_LIBADD = $(top_builddir)/lib/avcodec/libavcodec.la
