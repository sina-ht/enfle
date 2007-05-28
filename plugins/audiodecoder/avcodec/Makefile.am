## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.6 2007/05/28 15:24:38 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)
CFLAGS += -Wno-error

PLUGIN_ID = audiodecoder/avcodec

libdir = $(enfle_audiodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = audiodecoder_avcodec.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/lib/avutil

audiodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
audiodecoder_avcodec_la_CFLAGS = -Dasm=__asm $(MORE_CFLAGS)
audiodecoder_avcodec_la_SOURCES = avcodec.c
audiodecoder_avcodec_la_LIBADD = $(top_builddir)/lib/avutil/libavutil.la $(top_builddir)/lib/avcodec/libavcodec.la @LIBADD_VORBIS@ @LIBADD_THEORA@ @LIBADD_ZLIB@
