## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Jun 23 15:43:08 2002.
# $Id: Makefile.am,v 1.4 2002/08/02 14:03:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = archiver/tar

libdir = $(enfle_archiver_plugindir)

lib_LTLIBRARIES = archiver_tar.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

archiver_tar_la_LDFLAGS = $(MORE_LDFLAGS) -module
archiver_tar_la_CFLAGS = $(MORE_CFLAGS)
archiver_tar_la_SOURCES = tar.c _tar.h
