## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Feb  7 01:04:41 2004.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/jasper

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_j2k.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_j2k_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_j2k_la_CFLAGS = $(MORE_CFLAGS)
loader_j2k_la_SOURCES = j2k.c
loader_j2k_la_LIBADD = $(top_builddir)/lib/j2k/libenflej2k.la
