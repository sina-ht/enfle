## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# $Id: Makefile.am,v 1.1 2001/04/18 05:43:31 sian Exp $

libdir = $(enfle_saver_plugindir)

noinst_LTLIBRARIES = libarith.la
libarith_la_SOURCES = \
	arithcoder.h arithcoder_arith.c arithcoder_arith.h \
	arithmodel.h arithmodel_order_zero.c arithmodel_order_zero.h \
	arithmodel_utils.c \
	bitstream.c bitstream.h

lib_LTLIBRARIES = \
	saver_glic.la \
	vmpm_decompose_normal.la \
	vmpm_decompose_token.la \
	vmpm_decompose_token2.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(HOME)/work/m1/arith/src/libarith

saver_glic_la_LDFLAGS = -avoid-version -module -export-dynamic
saver_glic_la_SOURCES = \
	glic.c \
	vmpm_decompose.c vmpm_decompose.h \
	vmpm_error.c vmpm_error.h

COMMON_FILES = ipow.c vmpm_hash.c vmpm_hash.h

vmpm_decompose_normal_la_LIBADD = libarith.la
vmpm_decompose_normal_la_LDFLAGS = -avoid-version -module
vmpm_decompose_normal_la_SOURCES = vmpm_decompose_normal.c $(COMMON_FILES)
vmpm_decompose_token_la_LIBADD = libarith.la
vmpm_decompose_token_la_LDFLAGS = -avoid-version -module
vmpm_decompose_token_la_SOURCES = vmpm_decompose_token.c $(COMMON_FILES)
vmpm_decompose_token2_la_LIBADD = libarith.la
vmpm_decompose_token2_la_LDFLAGS = -avoid-version -module
vmpm_decompose_token2_la_SOURCES = vmpm_decompose_token2.c $(COMMON_FILES)