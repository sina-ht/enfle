## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/25 17:27:17 sian Exp $

libdir = $(enfle_effect_plugindir)

lib_LTLIBRARIES = effect_gamma.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

effect_gamma_la_LDFLAGS = -avoid-version -module
effect_gamma_la_SOURCES = gamma.c
effect_gamma_la_LIBADD = -lm
