## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2000/12/27 23:26:44 sian Exp $

CFLAGS += -DENFLE_PLUGINDIR=\"$(enfle_plugindir)\" -DENFLE_DATADIR=\"$(enfle_datadir)\"
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

bin_PROGRAMS = enfle
enfle_SOURCES = \
	enfle.c

enfle_LDFLAGS = -export-dynamic
enfle_LDADD = $(top_builddir)/lib/enfle/libenfle.la

EXTRA_DIST = enfle.rc
enfleconfdir = $(enfle_datadir)
enfleconf_DATA = enfle.rc
