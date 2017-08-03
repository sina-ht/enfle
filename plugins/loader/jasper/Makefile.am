## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Mar 12 00:33:21 2004.

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/jasper

libdir = $(enfle_loader_plugindir)

if HAVE_JASPER
lib_LTLIBRARIES = loader_jasper.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_jasper_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_jasper_la_CFLAGS = $(MORE_CFLAGS)
loader_jasper_la_SOURCES = jasper.c
loader_jasper_la_LIBADD = @LIBADD_JASPER@ @LIBADD_JPEG@
