## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.2 2000/12/27 23:24:44 sian Exp $

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_normal.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle

ui_normal_la_LDFLAGS = -avoid-version -module
ui_normal_la_SOURCES = normal.c
