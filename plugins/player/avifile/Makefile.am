## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Dec 14 21:36:04 2003.
# $Id: Makefile.am,v 1.5 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = player/avifile

AM_CXXFLAGS = -D_REENTRANT
libdir = $(enfle_player_plugindir)

if HAVE_AVIFILE
lib_LTLIBRARIES = player_avifile.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib @INCLUDEADD_AVIFILE@

player_avifile_la_LDFLAGS = $(MORE_LDFLAGS) -module
player_avifile_la_CFLAGS = $(MORE_CFLAGS)
player_avifile_la_SOURCES = avifile.cpp config.h
player_avifile_la_LIBADD = @LIBADD_AVIFILE@ @LIBADD_STDCC@
