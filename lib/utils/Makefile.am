## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.9 2001/10/06 16:01:09 sian Exp $

noinst_LTLIBRARIES = libenfleutils.la
INCLUDES = -I$(top_srcdir)

if HAVE_PTHREAD
thread_libs = @LIBADD_PTHREAD@
else
thread_libs =
endif

iconv_libs = @LIBADD_ICONV@

libenfleutils_la_LIBADD = $(thread_libs) $(iconv_libs) $(top_builddir)/compat/libcompat.la
libenfleutils_la_SOURCES = \
	misc.c misc.h \
	cpucaps.c cpucaps.h \
	timer.c timer.h timer_gettimeofday.c timer_gettimeofday.h timer_impl.h \
	pluginlist.c pluginlist.h \
	plugin.c plugin.h \
	libconfig.c libconfig.h \
	stdio-support.c stdio-support.h \
	libstring.c libstring.h \
	converter.c converter.h \
	fifo.c fifo.h \
	hash.c hash.h \
	dlist.c dlist.h
