## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:23 2000.
# $Id: Makefile.am,v 1.3 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = player_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/utils

player_ungif_la_LDFLAGS = -avoid-version -module
player_ungif_la_SOURCES = ungif.c
player_ungif_la_LIBADD = @LIBADD_UNGIF@
