## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.5 2001/02/12 13:11:31 sian Exp $

AM_CFLAGS = -DENFLE_PLUGINDIR=\"$(enfle_plugindir)\" -DENFLE_DATADIR=\"$(enfle_datadir)\"
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = enfle
enfle_SOURCES = \
	enfle.c

enfle_LDFLAGS = -export-dynamic
enfle_LDADD = $(top_builddir)/lib/enfle/libenfle.la

EXTRA_DIST = enfle.rc
enfleconfdir = $(enfle_datadir)
enfleconf_DATA = enfle.rc
