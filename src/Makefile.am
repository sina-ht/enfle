## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

AM_CFLAGS = -DENFLE_PLUGINDIR=\"$(enfle_plugindir)\" -DENFLE_DATADIR=\"$(enfle_datadir)\"
AM_CPPFLAG = -I$(top_srcdir) -I$(top_srcdir)/lib
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
install: install-am
	ENFLE_PLUGINDIR=$(enfle_plugindir) $(top_builddir)/src/enfle -N
