## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:24:56 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_loader_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = loader_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_ungif_la_LDFLAGS = -avoid-version -module
loader_ungif_la_SOURCES = ungif.c
loader_ungif_la_LIBADD = @LIBADD_UNGIF@
