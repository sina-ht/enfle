## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = effect/gamma

libdir = $(enfle_effect_plugindir)

lib_LTLIBRARIES = effect_gamma.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

effect_gamma_la_LDFLAGS = $(MORE_LDFLAGS) -module
effect_gamma_la_CFLAGS = $(MORE_CFLAGS)
effect_gamma_la_SOURCES = gamma.c
effect_gamma_la_LIBADD = -lm
