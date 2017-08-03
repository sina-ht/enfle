## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = audiodecoder/mpglib

libdir = $(enfle_audiodecoder_plugindir)

lib_LTLIBRARIES = audiodecoder_mpglib.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

audiodecoder_mpglib_la_LDFLAGS = $(MORE_LDFLAGS) -module
audiodecoder_mpglib_la_CFLAGS = $(MORE_CFLAGS)
audiodecoder_mpglib_la_SOURCES = mpglib.c
audiodecoder_mpglib_la_LIBADD = $(top_builddir)/lib/mpglib/libenflempglib.la
