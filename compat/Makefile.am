## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/20 07:22:12 sian Exp $

noinst_LTLIBRARIES = libcompat.la
INCLUDES = -I$(top_srcdir)

if HAVE_FNMATCH
FNMATCH_SOURCE =
else
FNMATCH_SOURCE = fnmatch.c fnmatch.h
endif

libcompat_la_SOURCES = $(FNMATCH_SOURCE)
