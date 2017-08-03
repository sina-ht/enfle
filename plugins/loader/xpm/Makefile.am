## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:34:27 2003.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/xpm

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_xpm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_xpm_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_xpm_la_CFLAGS = $(MORE_CFLAGS)
loader_xpm_la_SOURCES = xpm.c
loader_xpm_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la
