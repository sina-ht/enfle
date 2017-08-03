## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue Feb 10 01:27:19 2004.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/libmpeg2

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_libmpeg2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg2_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_libmpeg2_la_CFLAGS = $(MORE_CFLAGS)
player_libmpeg2_la_SOURCES = libmpeg2.c
player_libmpeg2_la_LIBADD = \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la \
	@LIBADD_PTHREAD@
