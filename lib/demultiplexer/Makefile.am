## Process this file with automake to produce Makefile.in
# Makefile.am for enfle
# Last Modified: Sun Sep  2 23:24:27 2001.
# $Id: Makefile.am,v 1.3 2001/09/03 00:31:21 sian Exp $

if HAVE_PTHREAD
noinst_LTLIBRARIES = libdemultiplexer.la
endif

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)

libdemultiplexer_la_SOURCES = \
	demultiplexer.c demultiplexer.h \
	demultiplexer_mpeg.c demultiplexer_mpeg.h \
	libriff.c libriff.h \
	demultiplexer_avi.c demultiplexer_avi.h
