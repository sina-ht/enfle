## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:25:32 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_MNG
lib_LTLIBRARIES = player_mng.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

player_mng_la_LDFLAGS = -avoid-version -module
player_mng_la_SOURCES = mng.c
player_mng_la_LIBADD = @LIBADD_MNG@
