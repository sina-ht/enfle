## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Oct 12 19:13:29 2000.
# $Id: Makefile.am,v 1.2 2000/10/12 15:43:47 sian Exp $

if HAVE_UNGIF
lib_LTLIBRARIES = player_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/utils

player_ungif_la_LDFLAGS = -module
player_ungif_la_SOURCES = ungif.c
player_ungif_la_LIBADD = @LIBADD_UNGIF@
