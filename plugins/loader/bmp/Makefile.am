## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:36:47 2003.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/bmp

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_bmp.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_bmp_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_bmp_la_CFLAGS = $(MORE_CFLAGS)
loader_bmp_la_SOURCES = bmp.c
