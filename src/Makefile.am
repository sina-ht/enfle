## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.8 2004/03/11 15:11:06 sian Exp $

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
