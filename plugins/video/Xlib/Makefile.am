## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2000/10/20 18:09:03 sian Exp $

if HAVE_X11
lib_LTLIBRARIES = video_Xlib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/utils -I$(top_srcdir)/lib/enfle -I$(top_srcdir)/lib/X11

video_Xlib_la_LDFLAGS = -module
video_Xlib_la_SOURCES = Xlib.c
video_Xlib_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la
