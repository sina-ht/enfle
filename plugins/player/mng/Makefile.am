## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:59:34 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_player_plugindir)

if HAVE_MNG
lib_LTLIBRARIES = player_mng.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/utils

player_mng_la_LDFLAGS = -avoid-version -module
player_mng_la_SOURCES = mng.c
player_mng_la_LIBADD = @LIBADD_MNG@
