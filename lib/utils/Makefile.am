## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.5 2001/06/22 17:34:43 sian Exp $

noinst_LTLIBRARIES = libenfleutils.la
INCLUDES = -I$(top_srcdir)

libenfleutils_la_LIBADD = $(top_builddir)/compat/libcompat.la
libenfleutils_la_SOURCES = \
	misc.c misc.h \
	fifo.c fifo.h \
	timer.c timer.h timer_gettimeofday.c timer_gettimeofday.h timer_impl.h \
	pluginlist.c pluginlist.h \
	plugin.c plugin.h \
	libconfig.c libconfig.h \
	stdio-support.c stdio-support.h \
	libstring.c libstring.h \
	hash.c hash.h \
	dlist.c dlist.h
