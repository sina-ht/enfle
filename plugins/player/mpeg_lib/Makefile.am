## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Sep 22 13:26:08 2000.
# $Id: Makefile.am,v 1.1 2000/10/12 03:45:29 sian Exp $

if HAVE_MPEG_LIB
lib_LTLIBRARIES = player_mpeg_lib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

player_mpeg_lib_la_LDFLAGS = -module
player_mpeg_lib_la_SOURCES = mpeg_lib.c
player_mpeg_lib_la_LIBADD = @LIBADD_MPEG_LIB@
