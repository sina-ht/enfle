## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2001/01/06 23:56:06 sian Exp $

libdir = $(enfle_video_plugindir)

if HAVE_X11
lib_LTLIBRARIES = video_Xlib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

video_Xlib_la_LDFLAGS = -avoid-version -module
video_Xlib_la_SOURCES = Xlib.c
video_Xlib_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la @LIBADD_PTHREAD@
