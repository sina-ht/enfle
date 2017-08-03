## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = audio/oss

libdir = $(enfle_audio_plugindir)

if HAVE_OSS
lib_LTLIBRARIES = audio_oss.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audio_oss_la_LDFLAGS = $(MORE_LDFLAGS) -module
audio_oss_la_CFLAGS = $(MORE_CFLAGS)
audio_oss_la_SOURCES = oss.c
