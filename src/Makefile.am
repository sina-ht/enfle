## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

bin_PROGRAMS = enfle
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

EXTRA_DIST = enfle.rc

enfle_SOURCES = \
	enfle.c

enfle_LDFLAGS = -export-dynamic
enfle_LDADD = $(top_builddir)/lib/enfle/libenfle.la
