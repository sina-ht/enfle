## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Feb 15 02:13:41 2002.
# $Id: Makefile.am,v 1.1 2002/02/14 19:19:09 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_xbm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_xbm_la_LDFLAGS = -avoid-version -module
loader_xbm_la_SOURCES = xbm.c
