## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.5 2003/02/05 15:23:35 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = ui/normal

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_normal.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib #-I/usr/local/include/gtk-1.2 -I/usr/local/include/glib-1.2 -I/usr/local/lib/glib/include

ui_normal_la_LDFLAGS = $(MORE_LDFLAGS) -module
ui_normal_la_CFLAGS = $(MORE_CFLAGS)
ui_normal_la_LIBADD = $(top_builddir)/lib/X11/libenfleX11.la #@LIBADD_PTHREAD@ -L/usr/local/lib -L/usr/X11R6/lib -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -lm
ui_normal_la_SOURCES = \
	normal.c
#	gui_gtk.c gui_gtk.h gui.h \
#	eventnotifier.c eventnotifier.h \
#	lockable.h \
#	leftarrow.xpm  leftarrow2.xpm  rightarrow.xpm  rightarrow2.xpm
