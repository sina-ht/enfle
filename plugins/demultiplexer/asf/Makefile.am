## Process this file with automake to produce Makefile.in
# Makefile.am for enfle

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC -static"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = demultiplexer/asf

libdir = $(enfle_demultiplexer_plugindir)

if HAVE_ASF
lib_LTLIBRARIES = demultiplexer_asf.la
endif

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

demultiplexer_asf_la_LDFLAGS = $(MORE_LDFLAGS) -module
demultiplexer_asf_la_CFLAGS = $(MORE_CFLAGS)
if HAVE_ASF
demultiplexer_asf_la_SOURCES = asf.c
endif
demultiplexer_asf_la_LIBADD = $(top_builddir)/lib/enfle/libenfle.la
