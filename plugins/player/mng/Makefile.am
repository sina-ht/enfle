## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Nov 18 13:02:16 2000.
# $Id: Makefile.am,v 1.1 2000/11/20 12:57:16 sian Exp $

if HAVE_MNG
lib_LTLIBRARIES = player_mng.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/utils

player_mng_la_LDFLAGS = -module
player_mng_la_SOURCES = mng.c
player_mng_la_LIBADD = @LIBADD_MNG@
