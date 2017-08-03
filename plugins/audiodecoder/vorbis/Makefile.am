## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = audiodecoder/vorbis

libdir = $(enfle_audiodecoder_plugindir)

if HAVE_VORBIS
lib_LTLIBRARIES = audiodecoder_vorbis.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audiodecoder_vorbis_la_LDFLAGS = $(MORE_LDFLAGS) -module
audiodecoder_vorbis_la_CFLAGS = $(MORE_CFLAGS)
audiodecoder_vorbis_la_SOURCES = vorbis.c
audiodecoder_vorbis_la_LIBADD = @LIBADD_VORBIS@
