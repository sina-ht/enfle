## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = ui/wallpaper

libdir = $(enfle_ui_plugindir)

lib_LTLIBRARIES = ui_wallpaper.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

ui_wallpaper_la_LDFLAGS = $(MORE_LDFLAGS) -module
ui_wallpaper_la_CFLAGS = $(MORE_CFLAGS)
ui_wallpaper_la_SOURCES = wallpaper.c
ui_wallpaper_la_LIBADD = $(top_builddir)/lib/enfle/libenfle.la
