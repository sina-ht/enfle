## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/18 05:29:50 sian Exp $

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_convert.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

ui_convert_la_LDFLAGS = -avoid-version -module
ui_convert_la_SOURCES = convert.c
