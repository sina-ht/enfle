## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.7 2009/01/03 15:35:57 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)
AM_CFLAGS = -Wno-error

PLUGIN_ID = audiodecoder/avcodec

libdir = $(enfle_audiodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = audiodecoder_avcodec.la
libavcodec = $(top_builddir)/lib/avutil/libavutil.la $(top_builddir)/lib/avcodec/libavcodec/la
AVCODEC_HEADER = -I$(top_srcdir)/lib/avutil
endif
if USE_SYSTEM_AVCODEC
lib_LTLIBRARIES = audiodecoder_avcodec.la
libavcodec = -lavutil -lavcodec
USE_SYSTEM_AVCODEC = -DUSE_SYSTEM_AVCODEC
AVCODEC_HEADER = -I/usr/local/include
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib $(AVCODEC_HEADER)

audiodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
audiodecoder_avcodec_la_CFLAGS = -Dasm=__asm $(MORE_CFLAGS) $(USE_SYSTEM_AVCODEC)
audiodecoder_avcodec_la_SOURCES = avcodec.c
audiodecoder_avcodec_la_LIBADD = $(libavcodec) @LIBADD_VORBIS@ @LIBADD_THEORA@ @LIBADD_ZLIB@
