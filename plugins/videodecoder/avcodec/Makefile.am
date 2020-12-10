## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)
AM_CFLAGS = -Wno-error

PLUGIN_ID = videodecoder/avcodec

libdir = $(enfle_videodecoder_plugindir)

if USE_AVCODEC
lib_LTLIBRARIES = videodecoder_avcodec.la
endif
libavcodec = -lavutil -lavcodec
AVCODEC_HEADER = -I/usr/local/include

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib $(AVCODEC_HEADER)

videodecoder_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
videodecoder_avcodec_la_CFLAGS = -Dasm=__asm $(MORE_CFLAGS)
videodecoder_avcodec_la_SOURCES = avcodec.c
videodecoder_avcodec_la_LIBADD = $(libavcodec) @LIBADD_VORBIS@ @LIBADD_THEORA@ @LIBADD_ZLIB@
