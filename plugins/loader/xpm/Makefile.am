## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Feb 15 03:52:40 2002.
# $Id: Makefile.am,v 1.1 2002/02/14 19:19:21 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_xpm.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_xpm_la_LDFLAGS = -avoid-version -module
loader_xpm_la_SOURCES = xpm.c
loader_xpm_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la
