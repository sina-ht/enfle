## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.7 2006/08/06 07:49:35 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = videodecoder/avcodec

libdir = $(enfle_videodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = videodecoder_avcodec.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/lib/avutil

videodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
videodecoder_avcodec_la_CFLAGS = -Dasm=__asm $(MORE_CFLAGS)
videodecoder_avcodec_la_SOURCES = avcodec.c
videodecoder_avcodec_la_LIBADD = $(top_builddir)/lib/avutil/libavutil.la $(top_builddir)/lib/avcodec/libavcodec.la @LIBADD_VORBIS@ @LIBADD_THEORA@ @LIBADD_ZLIB@
