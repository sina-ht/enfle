## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.9 2004/03/31 14:41:26 sian Exp $

AM_CFLAGS = -DENFLE_PLUGINDIR=\"$(enfle_plugindir)\" -DENFLE_DATADIR=\"$(enfle_datadir)\"
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = enfle
enfle_SOURCES = \
	enfle.c \
	getopt-support.c getopt-support.h

enfle_LDFLAGS = -export-dynamic
enfle_LDADD = @Static_plugin_objs@ $(top_builddir)/lib/enfle/libenfle.la
EXTRA_DIST = enfle.rc
enfleconfdir = $(enfle_datadir)
enfleconf_DATA = enfle.rc

.PHONY: install
install:
	ENFLE_PLUGINDIR=$(enfle_plugindir) $(top_build)/src/enfle -N
