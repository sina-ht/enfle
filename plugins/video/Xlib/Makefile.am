## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.5 2003/02/05 15:23:35 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = video/Xlib

libdir = $(enfle_video_plugindir)

if HAVE_X11
lib_LTLIBRARIES = video_Xlib.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

video_Xlib_la_LDFLAGS = $(MORE_LDFLAGS) -module
video_Xlib_la_CFLAGS = $(MORE_CFLAGS)
video_Xlib_la_SOURCES = Xlib.c Xlib.h
video_Xlib_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la @LIBADD_PTHREAD@
