## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2001/06/24 20:58:43 sian Exp $

noinst_LTLIBRARIES = libcompat.la
INCLUDES = -I$(top_srcdir)
EXTRA_DIST = fnmatch.c fnmatch.h

if HAVE_FNMATCH
FNMATCH_SOURCE =
else
FNMATCH_SOURCE = fnmatch.c fnmatch.h
endif

libcompat_la_SOURCES = $(FNMATCH_SOURCE)
