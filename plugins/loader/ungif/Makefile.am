## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:34:37 2003.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/ungif

libdir = $(enfle_loader_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = loader_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_ungif_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_ungif_la_CFLAGS = $(MORE_CFLAGS)
loader_ungif_la_SOURCES = ungif.c
loader_ungif_la_LIBADD = @LIBADD_UNGIF@
