## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.4 2001/08/26 00:55:56 sian Exp $

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_normal.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib #-I/usr/local/include/gtk-1.2 -I/usr/local/include/glib-1.2 -I/usr/local/lib/glib/include

ui_normal_la_LDFLAGS = -avoid-version -module
ui_normal_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la #@LIBADD_PTHREAD@ -L/usr/local/lib -L/usr/X11R6/lib -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
ui_normal_la_SOURCES = \
	normal.c
#	gui_gtk.c gui_gtk.h gui.h \
#	eventnotifier.c eventnotifier.h \
#	lockable.h \
#	leftarrow.xpm  leftarrow2.xpm  rightarrow.xpm  rightarrow2.xpm
