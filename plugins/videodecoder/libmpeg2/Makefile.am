## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = videodecoder/libmpeg2

libdir = $(enfle_videodecoder_plugindir)

lib_LTLIBRARIES = videodecoder_libmpeg2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

videodecoder_libmpeg2_la_LDFLAGS = $(MORE_LDFLAGS) -module
videodecoder_libmpeg2_la_CFLAGS = $(MORE_CFLAGS)
videodecoder_libmpeg2_la_SOURCES = libmpeg2.c
videodecoder_libmpeg2_la_LIBADD = $(top_builddir)/lib/mpeg2/libenflempeg2.la
