## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Fri Feb  2 11:15:10 2001.
# $Id: Makefile.am,v 1.1 2001/02/02 16:41:21 sian Exp $

libdir = $(enfle_loader_plugindir)

lib_LTLIBRARIES = loader_pcx.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

loader_pcx_la_LDFLAGS = -avoid-version -module
loader_pcx_la_SOURCES = pcx.c
