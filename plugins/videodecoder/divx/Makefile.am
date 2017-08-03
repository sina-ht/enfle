## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = videodecoder/divx

libdir = $(enfle_videodecoder_plugindir)

if HAVE_LIBDIVXDECORE
lib_LTLIBRARIES = videodecoder_divx.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

videodecoder_divx_la_LDFLAGS = $(MORE_LDFLAGS) -module
videodecoder_divx_la_CFLAGS = $(MORE_CFLAGS)
videodecoder_divx_la_SOURCES = divx.c
videodecoder_divx_la_LIBADD = @LIBADD_DIVXDECORE@
