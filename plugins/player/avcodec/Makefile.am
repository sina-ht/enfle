## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Tue Feb 10 01:27:07 2004.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/avcodec

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_avcodec.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_avcodec_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_avcodec_la_CFLAGS = $(MORE_CFLAGS)
player_avcodec_la_SOURCES = avcodec.c
player_avcodec_la_LIBADD = \
	@LIBADD_PTHREAD@ \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la
