## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.15 2003/02/05 15:19:41 sian Exp $

STATIC_PLUGINS = @Static_plugins@
MORE_CFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-DSTATIC"; else echo -n ""; fi)
MORE_LDFLAGS = $(shell if echo $(STATIC_PLUGINS) | grep $(PLUGIN_ID) >/dev/null; then echo -n "-static"; else echo -n "-avoid-version"; fi)

PLUGIN_ID = saver/glic

libdir = $(enfle_saver_plugindir)

noinst_LTLIBRARIES = libarith.la
libarith_la_SOURCES = \
	arithcoder.h arithcoder_arith.c arithcoder_arith.h \
	arithmodel.h arithmodel_order_zero.c arithmodel_order_zero.h \
	arithmodel_utils.c arithmodel_utils.h \
	bitstream.c bitstream.h

lib_LTLIBRARIES = \
	saver_glic.la \
	vmpm_decompose_normal.la \
	vmpm_decompose_escfirst.la \
	vmpm_decompose_esc.la \
	vmpm_decompose_escb.la \
	vmpm_decompose_escc.la \
	vmpm_decompose_recur.la \
	vmpm_decompose_recur2.la \
	vmpm_decompose_recur3.la \
	vmpm_decompose_highlow.la \
	vmpm_decompose_highlow_esc.la \
	vmpm_decompose_highlow_escb.la \
	vmpm_decompose_token.la \
	vmpm_decompose_token2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib

saver_glic_la_LIBADD = -lm
saver_glic_la_LDFLAGS = $(MORE_LDFLAGS) -module
saver_glic_la_CFLAGS = $(MORE_CFLAGS)
saver_glic_la_SOURCES = \
	glic.c \
	predict.c predict.h \
	scan.c scan.h \
	floorlog2.c floorlog2.h \
	ipow.c ipow.h \
	expand.c expand.h \
	threshold.c threshold.h \
	vmpm_decompose.c vmpm_decompose.h \
	vmpm_error.c vmpm_error.h \
	vmpm.h

COMMON_FILES = ipow.c ipow.h vmpm_hash.c vmpm_hash.h vmpm_error.c vmpm_error.h

vmpm_decompose_normal_la_LIBADD = libarith.la
vmpm_decompose_normal_la_LDFLAGS = -avoid-version -module
vmpm_decompose_normal_la_SOURCES = vmpm_decompose_normal.c $(COMMON_FILES)
vmpm_decompose_escfirst_la_LIBADD = libarith.la
vmpm_decompose_escfirst_la_LDFLAGS = -avoid-version -module
vmpm_decompose_escfirst_la_SOURCES = vmpm_decompose_escfirst.c $(COMMON_FILES)
vmpm_decompose_esc_la_LIBADD = libarith.la
vmpm_decompose_esc_la_LDFLAGS = -avoid-version -module
vmpm_decompose_esc_la_SOURCES = vmpm_decompose_esc.c $(COMMON_FILES)
vmpm_decompose_escb_la_LIBADD = libarith.la
vmpm_decompose_escb_la_LDFLAGS = -avoid-version -module
vmpm_decompose_escb_la_SOURCES = vmpm_decompose_escb.c $(COMMON_FILES)
vmpm_decompose_escc_la_LIBADD = libarith.la
vmpm_decompose_escc_la_LDFLAGS = -avoid-version -module
vmpm_decompose_escc_la_SOURCES = vmpm_decompose_escc.c $(COMMON_FILES)
vmpm_decompose_recur_la_LIBADD = libarith.la
vmpm_decompose_recur_la_LDFLAGS = -avoid-version -module
vmpm_decompose_recur_la_SOURCES = vmpm_decompose_recur.c $(COMMON_FILES)
vmpm_decompose_recur2_la_LIBADD = libarith.la
vmpm_decompose_recur2_la_LDFLAGS = -avoid-version -module
vmpm_decompose_recur2_la_SOURCES = vmpm_decompose_recur2.c $(COMMON_FILES)
vmpm_decompose_recur3_la_LIBADD = libarith.la
vmpm_decompose_recur3_la_LDFLAGS = -avoid-version -module
vmpm_decompose_recur3_la_SOURCES = vmpm_decompose_recur3.c $(COMMON_FILES)
vmpm_decompose_highlow_la_LIBADD = libarith.la
vmpm_decompose_highlow_la_LDFLAGS = -avoid-version -module
vmpm_decompose_highlow_la_SOURCES = vmpm_decompose_highlow.c $(COMMON_FILES)
vmpm_decompose_highlow_esc_la_LIBADD = libarith.la
vmpm_decompose_highlow_esc_la_LDFLAGS = -avoid-version -module
vmpm_decompose_highlow_esc_la_SOURCES = vmpm_decompose_highlow_esc.c $(COMMON_FILES)
vmpm_decompose_highlow_escb_la_LIBADD = libarith.la
vmpm_decompose_highlow_escb_la_LDFLAGS = -avoid-version -module
vmpm_decompose_highlow_escb_la_SOURCES = vmpm_decompose_highlow_escb.c $(COMMON_FILES)
vmpm_decompose_token_la_LIBADD = libarith.la
vmpm_decompose_token_la_LDFLAGS = -avoid-version -module
vmpm_decompose_token_la_SOURCES = vmpm_decompose_token.c $(COMMON_FILES)
vmpm_decompose_token2_la_LIBADD = libarith.la
vmpm_decompose_token2_la_LDFLAGS = -avoid-version -module
vmpm_decompose_token2_la_SOURCES = vmpm_decompose_token2.c $(COMMON_FILES)
