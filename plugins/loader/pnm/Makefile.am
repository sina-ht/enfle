## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Apr 13 14:20:08 2001.
# $Id: Makefile.am,v 1.1 2001/04/18 05:27:12 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_pnm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_pnm_la_LDFLAGS = -avoid-version -module
loader_pnm_la_SOURCES = pnm.c
