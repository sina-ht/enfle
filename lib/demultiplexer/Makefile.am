## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Mon Sep  3 10:53:42 2001.
# $Id: Makefile.am,v 1.4 2001/09/03 02:01:27 sian Exp $

if HAVE_PTHREAD
noinst_LTLIBRARIES = libdemultiplexer.la
endif

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)

libdemultiplexer_la_SOURCES = \
	demultiplexer.c demultiplexer.h \
	demultiplexer_mpeg.c demultiplexer_mpeg.h \
	libriff.c libriff.h \
	demultiplexer_avi.c demultiplexer_avi.h demultiplexer_avi_private.h
