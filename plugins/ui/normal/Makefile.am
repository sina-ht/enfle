## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2001/01/05 16:22:00 sian Exp $

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_normal.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

ui_normal_la_LDFLAGS = -avoid-version -module
ui_normal_la_SOURCES = normal.c
