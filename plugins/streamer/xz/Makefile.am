## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Oct  2 22:38:48 2009.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = streamer/xz

libdir = $(enfle_streamer_plugindir)

if HAVE_XZLIB
lib_LTLIBRARIES = streamer_xz.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

streamer_xz_la_LDFLAGS = $(MORE_LDFLAGS) -module
streamer_xz_la_CFLAGS = $(MORE_CFLAGS)
streamer_xz_la_SOURCES = xz.c
streamer_xz_la_LIBADD = @LIBADD_XZLIB@
