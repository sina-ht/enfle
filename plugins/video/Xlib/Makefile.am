## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.4 2001/08/15 06:30:50 sian Exp $

libdir = $(enfle_video_plugindir)

if HAVE_X11
lib_LTLIBRARIES = video_Xlib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

video_Xlib_la_LDFLAGS = -avoid-version -module
video_Xlib_la_SOURCES = Xlib.c Xlib.h
video_Xlib_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la @LIBADD_PTHREAD@
