## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.3 2003/12/16 16:51:55 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = ui/convert

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_convert.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

ui_convert_la_LDFLAGS = $(MORE_LDFLAGS) -module
ui_convert_la_CFLAGS = $(MORE_CFLAGS)
ui_convert_la_SOURCES = convert.c
