## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Sep 22 13:26:08 2000.
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

if HAVE_UNGIF
lib_LTLIBRARIES = loader_ungif.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/enfle

loader_ungif_la_LDFLAGS = -module
loader_ungif_la_SOURCES = ungif.c
loader_ungif_la_LIBADD = @LIBADD_UNGIF@
