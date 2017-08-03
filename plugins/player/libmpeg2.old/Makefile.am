## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Jan  1 19:59:38 2004.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/libmpeg2old

libdir = $(enfle_player_plugindir)

lib_LTLIBRARIES = player_libmpeg2old.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_libmpeg2old_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_libmpeg2old_la_CFLAGS = $(MORE_CFLAGS)
player_libmpeg2old_la_SOURCES = libmpeg2.c libmpeg2_vo.c libmpeg2_vo.h
player_libmpeg2old_la_LIBADD = \
	-L$(top_builddir)/lib/mpeg2.old -lenflempeg2old \
	$(top_builddir)/lib/mpglib/libenflempglib.la \
	$(top_builddir)/lib/demultiplexer/libdemultiplexer.la \
	@LIBADD_PTHREAD@
