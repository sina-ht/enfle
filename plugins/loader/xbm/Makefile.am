## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 15:39:30 2002.
# $Id: Makefile.am,v 1.2 2002/08/03 05:08:39 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = loader/xbm

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_xbm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_xbm_la_LDFLAGS = $(MORE_LDFLAGS) -module
loader_xbm_la_CFLAGS = $(MORE_CFLAGS)
loader_xbm_la_SOURCES = xbm.c
