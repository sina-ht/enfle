## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Thu Dec 28 06:58:08 2000.
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_archiver_plugindir)

lib_LTLIBRARIES = archiver_tar.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

archiver_tar_la_LDFLAGS = -avoid-version -module
archiver_tar_la_SOURCES = tar.c _tar.h
