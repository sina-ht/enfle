## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/09/30 17:36:36 sian Exp $

if HAVE_X11
lib_LTLIBRARIES = ui_Xlib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/X11

ui_Xlib_la_LDFLAGS = -module
ui_Xlib_la_SOURCES = Xlib.c
ui_Xlib_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la
