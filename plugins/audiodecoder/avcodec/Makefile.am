## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2005/07/24 18:02:42 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = audiodecoder/avcodec

libdir = $(enfle_audiodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = audiodecoder_avcodec.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audiodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module -export-symbols-regex 'plugin_*'
audiodecoder_avcodec_la_CFLAGS = $(MORE_CFLAGS)
audiodecoder_avcodec_la_SOURCES = avcodec.c
audiodecoder_avcodec_la_LIBADD = $(top_builddir)/lib/avcodec/libavcodec.la @LIBADD_VORBIS@ @LIBADD_THEORA@ @LIBADD_ZLIB@
