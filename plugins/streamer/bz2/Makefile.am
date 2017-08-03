## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:35:32 2003.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = streamer/bz2

libdir = $(enfle_streamer_plugindir)

if HAVE_BZLIB
lib_LTLIBRARIES = streamer_bz2.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

streamer_bz2_la_LDFLAGS = $(MORE_LDFLAGS) -module
streamer_bz2_la_CFLAGS = $(MORE_CFLAGS)
streamer_bz2_la_SOURCES = bz2.c bz2.h
streamer_bz2_la_LIBADD = @LIBADD_BZLIB@
