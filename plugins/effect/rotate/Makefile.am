## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/18 05:23:57 sian Exp $

libdir = $(enfle_effect_plugindir)

lib_LTLIBRARIES = effect_rotate.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

effect_rotate_la_LDFLAGS = -avoid-version -module
effect_rotate_la_SOURCES = rotate.c
