## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:21 2001.
# $Id: Makefile.am,v 1.4 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = player_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_ungif_la_LDFLAGS = -avoid-version -module
player_ungif_la_SOURCES = ungif.c
player_ungif_la_LIBADD = @LIBADD_UNGIF@
