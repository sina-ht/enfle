## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:42 2001.
# $Id: Makefile.am,v 1.1 2001/01/06 23:50:08 sian Exp $

CXXFLAGS += -D_REENTRANT
libdir = $(enfle_player_plugindir)

if HAVE_AVIFILE
lib_LTLIBRARIES = player_avifile.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib @INCLUDEADD_AVIFILE@

player_avifile_la_LDFLAGS = -avoid-version -module
player_avifile_la_SOURCES = avifile.cpp
player_avifile_la_LIBADD = @LIBADD_AVIFILE@ @LIBADD_STDCC@
