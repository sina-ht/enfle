## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Wed Jan 24 08:43:59 2001.
# $Id: Makefile.am,v 1.2 2001/01/23 23:49:44 sian Exp $

AM_CXXFLAGS = -D_REENTRANT
libdir = $(enfle_player_plugindir)

if HAVE_AVIFILE
lib_LTLIBRARIES = player_avifile.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib @INCLUDEADD_AVIFILE@

player_avifile_la_LDFLAGS = -avoid-version -module
player_avifile_la_SOURCES = avifile.cpp
player_avifile_la_LIBADD = @LIBADD_AVIFILE@ @LIBADD_STDCC@
