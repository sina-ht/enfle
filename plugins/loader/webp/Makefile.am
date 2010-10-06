## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Wed Oct  6 20:42:24 2010.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/webp

libdir = $(enfle_loader_plugindir)

if HAVE_WEBP
lib_LTLIBRARIES = loader_webp.la
endif
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_webp_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_webp_la_CFLAGS = $(MORE_CFLAGS)
loader_webp_la_SOURCES = webp.c
loader_webp_la_LIBADD = @LIBADD_WEBP@
