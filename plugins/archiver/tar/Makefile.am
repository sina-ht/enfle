## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sat Jan  6 01:24:19 2001.
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_archiver_plugindir)

lib_LTLIBRARIES = archiver_tar.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

archiver_tar_la_LDFLAGS = -avoid-version -module
archiver_tar_la_SOURCES = tar.c _tar.h
