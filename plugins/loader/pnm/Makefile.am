## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:34:42 2003.
# $Id: Makefile.am,v 1.3 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/pnm

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_pnm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_pnm_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_pnm_la_CFLAGS = $(MORE_CFLAGS)
loader_pnm_la_SOURCES = pnm.c
