## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Wed Feb  5 22:35:52 2003.
# $Id: Makefile.am,v 1.2 2003/02/05 15:23:35 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = archiver/arc

libdir = $(enfle_archiver_plugindir)

if HAVE_LIBARC
lib_LTLIBRARIES = archiver_arc.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

archiver_arc_la_LDFLAGS = $(MORE_LDFLAGS) -module
archiver_arc_la_CFLAGS = $(MORE_CFLAGS)
archiver_arc_la_SOURCES = arc.c
archiver_arc_la_LIBADD = @LIBADD_LIBARC@
