## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:58:58 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_loader_plugindir)

if HAVE_UNGIF
lib_LTLIBRARIES = loader_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_ungif_la_LDFLAGS = -avoid-version -module
loader_ungif_la_SOURCES = ungif.c
loader_ungif_la_LIBADD = @LIBADD_UNGIF@
