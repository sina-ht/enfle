## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Mar 19 00:30:04 2004.
# $Id: Makefile.am,v 1.3 2004/03/24 14:56:17 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/gif

libdir = $(enfle_loader_plugindir)

if HAVE_GIF
lib_LTLIBRARIES = loader_gif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_gif_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_gif_la_CFLAGS = $(MORE_CFLAGS)
if HAVE_GIF
loader_gif_la_SOURCES = gif.c giflib.c giflib.h gifbitio.c gifbitio.h gifimage.c gifext.c
endif
